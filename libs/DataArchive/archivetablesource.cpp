#include "archivetablesource.h"
#include "archivereader.h"

#include <algorithm>
#include <cstdint>
#include <limits>

namespace qds
{

ArchiveTableSource::ArchiveTableSource(
  ArchiveReader& reader)
  : m_reader(reader)
{
}

bool ArchiveTableSource::readBlock(
  Timestamp experimentStart,
  uint32_t frequency,
  uint64_t firstRow,
  uint32_t rowCount,
  const std::vector<SignalId>& channels,
  ArchiveTableBlock& block,
  const std::atomic_bool* cancel)
{
  block = {};

  if (!m_reader.isOpen() ||
      frequency == 0 ||
      frequency > 1'000'000 ||
      1'000'000 % frequency != 0 ||
      rowCount == 0)
  {
    return false;
  }

  constexpr uint64_t MicrosecondsPerSecond = 1'000'000;
  constexpr std::size_t ReadBlockSize = 1024;

  const uint64_t interval =
    MicrosecondsPerSecond / frequency;

  const uint64_t maxValue =
    std::numeric_limits<uint64_t>::max();

  if (firstRow > maxValue / interval)
    return false;

  const uint64_t offset = firstRow * interval;

  if (experimentStart.value > maxValue - offset)
    return false;

  const uint64_t firstTime =
    experimentStart.value + offset;

  if (rowCount > (maxValue - firstTime) / interval)
    return false;

  const uint64_t lastTime =
    firstTime + uint64_t(rowCount) * interval;

  block.firstRow = firstRow;
  block.frequency = frequency;
  block.channels = channels;

  block.values.resize(channels.size());

  for (auto& values : block.values)
    values.resize(rowCount);

  struct ChannelLocation
  {
    std::size_t fileIndex;
    std::size_t channelIndex;
    std::size_t column;
  };

  std::vector<ChannelLocation> locations;
  locations.reserve(channels.size());

  for (std::size_t column = 0;
       column < channels.size();
       ++column)
  {
    if (cancel && cancel->load())
      return false;

    const auto location =
      m_reader.findSignal(channels[column]);

    if (!location)
      return false;

    locations.push_back({
      location->fileIndex,
      location->channelIndex,
      column
    });
  }

  std::ranges::sort(
    locations,
    {},
    &ChannelLocation::fileIndex);


  for (std::size_t begin = 0;
       begin < locations.size();)
  {
    if (cancel && cancel->load())
      return false;

    const auto fileIndex =
      locations[begin].fileIndex;

    std::size_t end = begin + 1;

    while (end < locations.size() &&
           locations[end].fileIndex == fileIndex)
    {
      ++end;
    }

    const auto* header =
      m_reader.fileHeader(fileIndex);

    if (!header)
      return false;

    for (std::size_t i = begin; i < end; ++i)
    {
      if (locations[i].channelIndex >= header->channelCount)
        return false;
    }

    if (header->sampleFrequency == 0 ||
        header->sampleFrequency > MicrosecondsPerSecond)
      return false;

    // удержание последнего отсчёта (Zero-Order Hold)
    const uint64_t sourceInterval =
      MicrosecondsPerSecond / header->sampleFrequency;

    const bool holdValues =
      header->sampleFrequency < frequency;

    uint64_t recordIndex = 0;

    // Для удержания нужно найти последний отсчёт до начала блока
    // Так мы захватим возможный предыдущий отсчёт
    const uint64_t searchTime =
      holdValues && firstTime >= sourceInterval
        ? firstTime - sourceInterval
        : 0;

    if (!m_reader.findRecord(
          fileIndex,
          Timestamp{searchTime},
          recordIndex))
    {
      return false;
    }

    const std::size_t groupSize = end - begin;

    // отдельная ветка удержания
    if (holdValues)
    {
      std::vector<ArchiveSample> samples;

      std::optional<ArchiveSample> previous;

      uint64_t row = 0;
      bool finished = false;

      while (recordIndex < header->recordCount &&
             !finished)
      {
        if (cancel && cancel->load())
          return false;

        if (!m_reader.readRecords(
              fileIndex,
              recordIndex,
              ReadBlockSize,
              samples))
        {
          return false;
        }

        if (samples.empty())
          break;

        for (const auto& sample : samples)
        {
          const uint64_t sampleTime =
            sample.timestamp.value;

          if (sampleTime >= lastTime)
          {
            finished = true;
            break;
          }

          if (previous)
          {
            const uint64_t validUntil =
              previous->timestamp.value >
                  maxValue - sourceInterval
                ? maxValue
                : previous->timestamp.value +
                    sourceInterval;

            const uint64_t endTime =
              std::min(sampleTime, validUntil);

            while (row < rowCount)
            {
              const uint64_t rowTime =
                firstTime + row * interval;

              if (rowTime >= endTime)
                break;

              if (rowTime >= previous->timestamp.value)
              {
                for (std::size_t i = begin; i < end; ++i)
                {
                  const auto& location = locations[i];

                  if (location.channelIndex >=
                      previous->values.size())
                  {
                    return false;
                  }

                  block.values[location.column][row] =
                    previous->values[location.channelIndex];
                }
              }

              ++row;
            }
          }

          previous = sample;
        }

        recordIndex += samples.size();
      }

      // Заполняем оставшиеся строки последним отсчётом,
      // но не дольше одного периода источника.
      if (previous)
      {
        const uint64_t validUntil =
          previous->timestamp.value >
              maxValue - sourceInterval
            ? maxValue
            : previous->timestamp.value +
                sourceInterval;

        while (row < rowCount)
        {
          const uint64_t rowTime =
            firstTime + row * interval;

          if (rowTime >= validUntil)
            break;

          if (rowTime >= previous->timestamp.value)
          {
            for (std::size_t i = begin; i < end; ++i)
            {
              const auto& location = locations[i];

              if (location.channelIndex >=
                  previous->values.size())
              {
                return false;
              }

              block.values[location.column][row] =
                previous->values[location.channelIndex];
            }
          }

          ++row;
        }
      }

      begin = end;
      continue;
    }

    std::vector<std::vector<double>> sums(
      groupSize,
      std::vector<double>(rowCount, 0.0));

    std::vector<std::vector<uint32_t>> counts(
      groupSize,
      std::vector<uint32_t>(rowCount, 0));

    std::vector<ArchiveSample> samples;

    bool finished = false;

    while (recordIndex < header->recordCount &&
           !finished)
    {
      if (cancel && cancel->load())
        return false;

      if (!m_reader.readRecords(
            fileIndex,
            recordIndex,
            ReadBlockSize,
            samples))
      {
        return false;
      }

      if (samples.empty())
        break;

      for (const auto& sample : samples)
      {
        if (sample.timestamp.value >= lastTime)
        {
          finished = true;
          break;
        }

        if (sample.timestamp.value < firstTime)
          continue;

        const uint64_t row =
          (sample.timestamp.value - firstTime) /
          interval;

        if (row >= rowCount)
          continue;

        for (std::size_t i = begin; i < end; ++i)
        {
          const auto& location = locations[i];

          if (location.channelIndex >= sample.values.size())
            return false;

          const std::size_t groupColumn = i - begin;

          sums[groupColumn][row] +=
            static_cast<double>(
              sample.values[location.channelIndex]);

          ++counts[groupColumn][row];
        }
      }

      recordIndex += samples.size();
    }

    for (std::size_t i = begin; i < end; ++i)
    {
      const auto& location = locations[i];
      const std::size_t groupColumn = i - begin;

      for (uint32_t row = 0; row < rowCount; ++row)
      {
        if (counts[groupColumn][row] == 0)
          continue;

        block.values[location.column][row] =
          sums[groupColumn][row] /
          counts[groupColumn][row];
      }
    }

    begin = end;
  }

  return true;
}

}