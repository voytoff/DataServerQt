#include "archivesignalsource.h"

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


}