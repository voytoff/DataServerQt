#include "archivesignalsource.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace qds
{

ArchiveSignalSource::ArchiveSignalSource(
  ArchiveReader& reader)
  : m_reader(reader)
{
}

std::optional<ArchiveSignalLocation>
ArchiveSignalSource::findSignal(
  SignalId signalId) const
{
  if (!m_reader.isOpen())
    return std::nullopt;

  const auto& description =
    m_reader.description();

  for (std::size_t fileIndex = 0;
       fileIndex < description.files.size();
       ++fileIndex)
  {
    const auto& file =
      description.files[fileIndex];

    for (const auto& signal : file.signalIds)
    {
      if (signal.id == signalId)
      {
        return ArchiveSignalLocation{
          fileIndex,
          signal.index
        };
      }
    }
  }

  return std::nullopt;
}

bool ArchiveSignalSource::readRange(
  SignalId signalId,
  Timestamp firstTimestamp,
  Timestamp lastTimestamp,
  std::size_t maxRecords, // ограничивает общее число возвращаемых точек, а не размер блока чтения
  ArchiveSignalPoints& points)
{
  points.clear();

  if (firstTimestamp > lastTimestamp || maxRecords == 0)
    return false;

  const auto location = findSignal(signalId);

  if (!location)
    return false;

  const auto* header =
    m_reader.fileHeader(location->fileIndex);

  if (!header)
    return false;

  if (location->channelIndex >= header->channelCount)
    return false;

  uint64_t recordIndex = 0;

  if (!m_reader.findRecord(
        location->fileIndex,
        firstTimestamp,
        recordIndex))
  {
    return false;
  }

  constexpr std::size_t BlockSize = 4096;

  std::vector<ArchiveSample> samples;

  while (recordIndex < header->recordCount &&
         points.size() < maxRecords)
  {
    const auto remaining =
      maxRecords - points.size();

    const auto count =
      std::min(BlockSize, remaining);

    if (!m_reader.readRecords(
          location->fileIndex,
          recordIndex,
          count,
          samples))
    {
      points.clear();
      return false;
    }

    if (samples.empty())
      break;

    for (const auto& sample : samples)
    {
      if (sample.timestamp > lastTimestamp)
        return true;

      if (location->channelIndex >= sample.values.size())
      {
        points.clear();
        return false;
      }

      points.push_back({
        .timestamp = sample.timestamp,
        .wallTime = sample.wallTime,
        .frameNumber = sample.frameNumber,
        .value = static_cast<double>(
          sample.values[location->channelIndex])
      });

      if (points.size() >= maxRecords)
        break;
    }

    recordIndex += samples.size();
  }

  return true;
}

bool ArchiveSignalSource::readPlot(
  SignalId signalId,
  Timestamp firstTimestamp,
  Timestamp lastTimestamp,
  std::size_t maxPoints,
  ArchiveSignalPoints& points,
  const std::atomic_bool *cancel)
{
  points.clear();

  if (firstTimestamp > lastTimestamp || maxPoints < 2)
    return false;

  const auto location = findSignal(signalId);

  if (!location)
    return false;

  const auto* header =
    m_reader.fileHeader(location->fileIndex);

  if (!header ||
      location->channelIndex >= header->channelCount)
    return false;

  uint64_t recordIndex = 0;

  if (!m_reader.findRecord(
        location->fileIndex,
        firstTimestamp,
        recordIndex))
    return false;

  const std::size_t bucketCount = maxPoints / 2;

  // Расчёты выполняем в целых микросекундах.
  const uint64_t duration =
    lastTimestamp.value - firstTimestamp.value;

  struct Bucket
  {
    bool hasValue = false;
    ArchiveSignalPoint minimum;
    ArchiveSignalPoint maximum;
  };

  std::vector<Bucket> buckets(bucketCount);

  constexpr std::size_t BlockSize = 4096;

  std::vector<ArchiveSample> samples;

  while (recordIndex < header->recordCount)
  {
    if (cancel && cancel->load(std::memory_order_relaxed))
    {
      points.clear();
      return false;
    }

    if (!m_reader.readRecords(
          location->fileIndex,
          recordIndex,
          BlockSize,
          samples))
    {
      points.clear();
      return false;
    }

    if (samples.empty())
      break;

    bool finished = false;

    for (const auto& sample : samples)
    {
      if (cancel && cancel->load(std::memory_order_relaxed))
      {
        points.clear();
        return false;
      }

      if (sample.timestamp > lastTimestamp)
      {
        finished = true;
        break;
      }

      if (location->channelIndex >= sample.values.size())
      {
        points.clear();
        return false;
      }

      const double value =
        static_cast<double>(
          sample.values[location->channelIndex]);

      if (!std::isfinite(value))
        continue;

      const uint64_t offset =
        sample.timestamp.value - firstTimestamp.value;

      // Индекс временного интервала без переполнения
      // при умножении больших uint64_t.
      const std::size_t bucketIndex =
        duration == 0
          ? 0
          : std::min(
              bucketCount - 1,
              static_cast<std::size_t>(
                static_cast<long double>(offset) *
                bucketCount /
                (static_cast<long double>(duration) + 1.0L)));

      ArchiveSignalPoint point{
        .timestamp = sample.timestamp,
        .wallTime = sample.wallTime,
        .frameNumber = sample.frameNumber,
        .value = value
      };

      auto& bucket = buckets[bucketIndex];

      if (!bucket.hasValue)
      {
        bucket.hasValue = true;
        bucket.minimum = point;
        bucket.maximum = point;
      }
      else
      {
        if (point.value < bucket.minimum.value)
          bucket.minimum = point;

        if (point.value > bucket.maximum.value)
          bucket.maximum = point;
      }
    }

    if (finished)
      break;

    recordIndex += samples.size();
  }

  points.reserve(maxPoints);

  for (const auto& bucket : buckets)
  {
    if (!bucket.hasValue)
      continue;

    if (bucket.minimum.timestamp == bucket.maximum.timestamp)
    {
      points.push_back(bucket.minimum);
    }
    else if (bucket.minimum.timestamp < bucket.maximum.timestamp)
    {
      points.push_back(bucket.minimum);
      points.push_back(bucket.maximum);
    }
    else
    {
      points.push_back(bucket.maximum);
      points.push_back(bucket.minimum);
    }
  }

  return true;
}


}