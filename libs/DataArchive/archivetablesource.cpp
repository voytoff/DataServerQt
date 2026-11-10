#include "archivetablesource.h"
#include "archivereader.h"

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
  const std::vector<SignalId> &channels,
  ArchiveTableBlock &block,
  const std::atomic_bool *cancel)
{
  block.values.clear();

  if (rowCount == 0)
    return false;

  const auto firstTimestamp =  Timestamp{firstRow * frequency * 1'000};
  const auto lastTimestamp =  Timestamp{(firstRow + rowCount) * frequency * 1'000};

  for (const auto signalId : channels)
  {
    std::vector<std::optional<double>> array;
    block.values.push_back(array);

    const auto location = m_reader.findSignal(signalId);

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

    constexpr std::size_t BlockSize = 1024;

    std::vector<ArchiveSample> samples;

    while (recordIndex < header->recordCount &&
           block.values.size() < rowCount)
    {
      const auto remaining =
        rowCount - block.values.size();

      const auto count =
        std::min(BlockSize, remaining);

      if (!m_reader.readRecords(
            location->fileIndex,
            recordIndex,
            count,
            samples))
      {
        block.values.clear();
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
          block.values.clear();
          return false;
        }

        array.push_back(
          static_cast<double>(
              sample.values[location->channelIndex])
          );

        if (channels.size() >= rowCount)
          break;
      }

      recordIndex += samples.size();
    }
  }
  return true;
}

}