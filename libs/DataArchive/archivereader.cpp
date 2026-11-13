#include "archivereader.h"

#include <algorithm>
#include <stdexcept>
#include "archivedescriptionreader.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QByteArray>

namespace qds
{

bool ArchiveReader::open(
  const std::filesystem::path& directory)
{
  if (m_open)
    return false;

  ArchiveDescription tmp;

  ArchiveDescriptionReader reader;

  if (!reader.read(
        directory / DescriptionFileName,
        tmp))
  {
    return false;
  }

  std::vector<File> tmpFiles;
  tmpFiles.reserve(tmp.files.size());

  for (const auto& description : tmp.files)
  {
    File file;

    file.description = description;
    file.path = directory / description.name;

    tmpFiles.push_back(
      std::move(file));
  }

  m_directory = directory;
  m_description = std::move(tmp);
  m_files = std::move(tmpFiles);

  m_open = true;

  return true;
}

void ArchiveReader::close() noexcept
{
  for (auto& file : m_files)
    file.archive.close();

  m_files.clear();
  m_description = {};
  m_directory.clear();
  m_open = false;
}

bool ArchiveReader::isOpen() const noexcept
{
  return m_open;
}

const ArchiveDescription &ArchiveReader::description() const noexcept
{
  return m_description;
}

std::size_t ArchiveReader::fileCount() const noexcept
{
  return m_files.size();
}

const ArchiveFileDescription& ArchiveReader::fileDescription(
  std::size_t fileIndex) const
{
  if (!m_open)
    throw std::logic_error("ArchiveReader is not open");

  if (fileIndex >= m_files.size())
    throw std::out_of_range("ArchiveReader: invalid file index");

  return m_files[fileIndex].description;
}

const DataFileHeader* ArchiveReader::fileHeader(
  std::size_t fileIndex)
{
  if (!m_open || fileIndex >= m_files.size())
    return nullptr;

  if (!ensureOpen(fileIndex))
    return nullptr;

  return &m_files[fileIndex].archive.header();
}

bool ArchiveReader::read(
  std::size_t fileIndex,
  ArchiveSample& sample)
{
  if (!m_open || fileIndex >= m_files.size())
    return false;

  if (!ensureOpen(fileIndex))
    return false;

  auto& file = m_files[fileIndex].archive;

  const auto channelCount =
    file.header().channelCount;

  SampleRecordHeader header;

  if (!file.readObject(header))
    return false;

  ArchiveSample result;

  result.frameNumber = FrameNumber{header.frameNumber};
  result.timestamp = Timestamp{header.timestamp};
  result.wallTime = WallClockTime{header.wallTime};

  result.values.resize(channelCount);

  if (!file.readArray(
        result.values.data(),
        result.values.size()))
  {
    return false;
  }

  sample = std::move(result);

  return true;
}

bool ArchiveReader::readFrame(
  std::size_t fileIndex,
  FrameNumber frameNumber,
  ArchiveSample& sample)
{
  uint64_t recordIndex = 0;

  if (!findRecord(
        fileIndex,
        frameNumber,
        recordIndex))
  {
    return false;
  }

  const auto* header =
    fileHeader(fileIndex);

  if (!header)
    return false;

  if (recordIndex >= header->recordCount)
    return false;

  ArchiveSample result;

  if (!readRecord(
        fileIndex,
        recordIndex,
        result))
  {
    return false;
  }

  if (result.frameNumber != frameNumber)
    return false;

  sample = std::move(result);

  return true;
}

bool ArchiveReader::readRecord(
  std::size_t fileIndex,
  uint64_t recordIndex,
  ArchiveSample& sample)
{
  uint64_t position = 0;

  if (!recordPosition(
        fileIndex,
        recordIndex,
        position))
  {
    return false;
  }

  auto& file = m_files[fileIndex].archive;

  if (!file.seek(position))
    return false;

  return read(fileIndex, sample);
}

bool ArchiveReader::findRecord(
  std::size_t fileIndex,
  FrameNumber frameNumber,
  uint64_t& recordIndex)
{
  if (!m_open || fileIndex >= m_files.size())
    return false;

  const auto* header = fileHeader(fileIndex);

  if (!header)
    return false;

  uint64_t left = 0;
  uint64_t right = header->recordCount;

  while (left < right)
  {
    const uint64_t middle =
      left + (right - left) / 2;

    SampleRecordHeader record;

    if (!readRecordHeader(
          fileIndex,
          middle,
          record))
    {
      return false;
    }

    if (record.frameNumber < frameNumber.value)
      left = middle + 1;
    else
      right = middle;
  }

  recordIndex = left;

  return true;
}

bool ArchiveReader::findRecord(
  std::size_t fileIndex,
  Timestamp timestamp,
  uint64_t& recordIndex)
{
  const auto* header =
    fileHeader(fileIndex);

  if (!header)
    return false;

  uint64_t left = 0;
  uint64_t right = header->recordCount;

  while (left < right)
  {
    const uint64_t middle =
      left + (right - left) / 2;

    SampleRecordHeader recordHeader;

    if (!readRecordHeader(
          fileIndex,
          middle,
          recordHeader))
    {
      return false;
    }

    if (recordHeader.timestamp < timestamp.value)
      left = middle + 1;
    else
      right = middle;
  }

  recordIndex = left;

  return true;
}

bool ArchiveReader::readRange(
  std::size_t fileIndex,
  FrameNumber firstFrame,
  FrameNumber lastFrame,
  std::vector<ArchiveSample>& samples)
{
  samples.clear();

  if (firstFrame.value > lastFrame.value)
    return false;

  uint64_t recordIndex = 0;

  if (!findRecord(
        fileIndex,
        firstFrame,
        recordIndex))
  {
    return false;
  }

  const auto* header = fileHeader(fileIndex);

  if (!header)
    return false;

  for (
    uint64_t i = recordIndex;
    i < header->recordCount;
    ++i)
  {
    ArchiveSample sample;

    if (!readRecord(fileIndex, i, sample))
      return false;

    if (sample.frameNumber.value > lastFrame.value)
      break;

    samples.push_back(std::move(sample));
  }

  return true;
}

bool ArchiveReader::readRange(
  std::size_t fileIndex,
  FrameNumber firstFrame,
  FrameNumber lastFrame,
  std::size_t maxRecords,
  std::vector<ArchiveSample>& samples)
{
  samples.clear();

  if (firstFrame.value > lastFrame.value)
    return false;

  if (maxRecords == 0)
    return false;

  uint64_t recordIndex = 0;

  if (!findRecord(
        fileIndex,
        firstFrame,
        recordIndex))
  {
    return false;
  }

  const auto* header = fileHeader(fileIndex);

  if (!header)
    return false;

  if (recordIndex >= header->recordCount)
    return true;

  uint64_t position = 0;

  if (!recordPosition(
        fileIndex,
        recordIndex,
        position))
  {
    return false;
  }

  auto& file = m_files[fileIndex].archive;

  if (!file.seek(position))
    return false;

  while (
    recordIndex < header->recordCount &&
    samples.size() < maxRecords)
  {
    ArchiveSample sample;

    if (!read(fileIndex, sample))
    {
      samples.clear();
      return false;
    }

    if (sample.frameNumber.value > lastFrame.value)
      break;

    samples.push_back(std::move(sample));
    ++recordIndex;
  }

  return true;
}

bool ArchiveReader::readRecords(
  std::size_t fileIndex,
  uint64_t firstRecord,
  std::size_t maxRecords,
  std::vector<ArchiveSample>& samples)
{
  samples.clear();

  if (maxRecords == 0)
    return false;

  const auto* header = fileHeader(fileIndex);

  if (!header)
    return false;

  if (firstRecord >= header->recordCount)
    return true;

  uint64_t position = 0;

  if (!recordPosition(
        fileIndex,
        firstRecord,
        position))
  {
    return false;
  }

  auto& file = m_files[fileIndex].archive;

  if (!file.seek(position))
    return false;

  const uint64_t count = std::min<uint64_t>(
    header->recordCount - firstRecord,
    maxRecords);

  for (uint64_t i = 0; i < count; ++i)
  {
    ArchiveSample sample;

    if (!read(fileIndex, sample))
    {
      samples.clear();
      return false;
    }

    samples.push_back(std::move(sample));
  }

  return true;
}

bool ArchiveReader::timeRange(
  Timestamp& firstTimestamp,
  Timestamp& lastTimestamp)
{
  if (!m_open)
    return false;

  bool initialized = false;

  uint64_t first = 0;
  uint64_t last = 0;

  for (std::size_t i = 0; i < m_files.size(); ++i)
  {
    const auto* header = fileHeader(i);

    if (!header)
      return false;

    if (header->recordCount == 0)
      continue;

    if (header->firstTimestamp > header->lastTimestamp)
      return false;

    if (!initialized)
    {
      first = header->firstTimestamp;
      last = header->lastTimestamp;
      initialized = true;
    }
    else
    {
      first = std::min(first, header->firstTimestamp);
      last = std::max(last, header->lastTimestamp);
    }
  }

  if (!initialized)
    return false;

  firstTimestamp = Timestamp{first};
  lastTimestamp = Timestamp{last};

  return true;
}

bool ArchiveReader::timeReference(
  Timestamp& timestamp,
  WallClockTime& wallTime)
{
  if (!m_open)
    return false;

  bool initialized = false;
  ArchiveSample earliest;

  for (std::size_t i = 0; i < m_files.size(); ++i)
  {
    const auto* header = fileHeader(i);

    if (!header)
      return false;

    if (header->recordCount == 0)
      continue;

    ArchiveSample sample;

    if (!readRecord(i, 0, sample))
      return false;

    if (!initialized ||
        sample.timestamp < earliest.timestamp)
    {
      earliest = std::move(sample);
      initialized = true;
    }
  }

  if (!initialized)
    return false;

  timestamp = earliest.timestamp;
  wallTime = earliest.wallTime;

  return true;
}

std::optional<ArchiveLocation>
ArchiveReader::findSignal(
  SignalId signalId) const
{
  if (!m_open)
    return std::nullopt;

  for (std::size_t fileIndex = 0;
       fileIndex < m_description.files.size();
       ++fileIndex)
  {
    const auto& file =
      m_description.files[fileIndex];

    for (const auto& signal : file.signalIds)
    {
      if (signal.id == signalId)
      {
        return ArchiveLocation{
          fileIndex,
          signal.index
        };
      }
    }
  }

  return std::nullopt;
}

bool ArchiveReader::ensureOpen(std::size_t fileIndex)
{
  auto& file = m_files[fileIndex];

  if (file.archive.isOpen())
    return file.archive.header().isValid();

  if (!file.archive.open(file.path, OpenMode::Read))
    return false;

  if (!file.archive.header().isValid())
  {
    file.archive.close();
    return false;
  }

  return true;
}

bool ArchiveReader::readRecordHeader(
  std::size_t fileIndex,
  uint64_t recordIndex,
  SampleRecordHeader& header)
{
  uint64_t position = 0;

  if (!recordPosition(
        fileIndex,
        recordIndex,
        position))
  {
    return false;
  }

  auto& file = m_files[fileIndex].archive;

  if (!file.seek(position))
    return false;

  return file.readObject(header);
}

bool ArchiveReader::recordPosition(
  std::size_t fileIndex,
  uint64_t recordIndex,
  uint64_t& position)
{
  if (!m_open || fileIndex >= m_files.size())
    return false;

  if (!ensureOpen(fileIndex))
    return false;

  auto& file = m_files[fileIndex].archive;
  const auto& header = file.header();

  if (recordIndex >= header.recordCount)
    return false;

  const uint64_t fileSize = file.fileSize();

  if (header.headerSize > fileSize)
    return false;

  const uint64_t available =
    fileSize - header.headerSize;

  if (header.recordSize > available)
    return false;

  // Проверяем индекс без умножения, чтобы
  // избежать переполнения uint64_t.
  if (
    recordIndex >
    (available - header.recordSize) /
      header.recordSize)
  {
    return false;
  }

  position =
    static_cast<uint64_t>(header.headerSize) +
    recordIndex *
      static_cast<uint64_t>(header.recordSize);

  return true;
}

}
