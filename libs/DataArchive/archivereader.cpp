#include "archivereader.h"
#include "archivedescriptionreader.h"
#include "archivesegment.h"

#include <algorithm>
#include <stdexcept>
#include <ranges>
#include <system_error>
#include <utility>
#include <limits>

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

  if (tmp.segmentFrameCount == 0 ||
      tmp.files.empty())
  {
    return false;
  }

  std::vector<File> tmpFiles;
  tmpFiles.reserve(tmp.files.size());

  std::vector<uint64_t> expectedSegments;

  for (const auto& description : tmp.files)
  {
    File file;
    file.description = description;

    std::vector<std::pair<
      uint64_t,
      std::filesystem::path>> paths;

    std::error_code ec;

    for (const auto& entry :
         std::filesystem::directory_iterator(
           directory, ec))
    {
      if (ec)
        return false;

      if (!entry.is_regular_file(ec))
      {
        if (ec)
          return false;

        continue;
      }

      const auto number =
        parseSegmentFileName(
          description.name,
          entry.path().filename());

      if (!number)
        continue;

      paths.emplace_back(
        *number,
        entry.path());
    }

    if (ec || paths.empty())
      return false;

    std::ranges::sort(
      paths,
      {},
      [](const auto& item)
      {
        return item.first;
      });

    for (std::size_t i = 1;
         i < paths.size();
         ++i)
    {
      if (paths[i].first !=
          paths[i - 1].first + 1)
      {
        return false;
      }
    }

    std::vector<uint64_t> numbers;
    numbers.reserve(paths.size());

    for (const auto& [number, path] : paths)
      numbers.push_back(number);

    if (expectedSegments.empty())
    {
      expectedSegments = std::move(numbers);
    }
    else if (expectedSegments != numbers)
    {
      return false;
    }

    file.segments.reserve(paths.size());

    uint64_t totalRecords = 0;

    bool firstSegment = true;
    bool hasRecords = false;

    std::optional<uint64_t> previousTimestamp;

    for (const auto& [number, path] : paths)
    {
      Segment segment;

      segment.number = number;
      segment.path = path;
      segment.firstRecord = totalRecords;

      if (!segment.archive.open(
            path,
            OpenMode::Read))
      {
        return false;
      }

      segment.header = segment.archive.header();

      // Проверяем соответствие описанию архива.
      if (segment.header.sampleFrequency !=
            description.frequency ||
          segment.header.channelCount !=
            description.signalIds.size())
      {
        return false;
      }

      // Проверяем физический размер файла.
      std::error_code ec;

      const uint64_t fileSize =
        std::filesystem::file_size(path, ec);

      if (ec)
        return false;

      const uint64_t headerSize =
        segment.header.headerSize;

      const uint64_t recordSize =
        segment.header.recordSize;

      if (fileSize < headerSize)
        return false;

      const uint64_t dataSize =
        fileSize - headerSize;

      if (recordSize == 0 ||
          dataSize % recordSize != 0 ||
          dataSize / recordSize !=
            segment.header.recordCount)
      {
        return false;
      }

      if (firstSegment)
      {
        file.header = segment.header;
        firstSegment = false;
      }
      else
      {
        if (segment.header.recordSize !=
              file.header.recordSize ||
            segment.header.sampleFrequency !=
              file.header.sampleFrequency ||
            segment.header.channelCount !=
              file.header.channelCount)
        {
          return false;
        }
      }

      if (segment.header.recordCount > 0)
      {
        if (segment.header.firstTimestamp >
            segment.header.lastTimestamp)
        {
          return false;
        }

        if (previousTimestamp &&
            segment.header.firstTimestamp <=
              *previousTimestamp)
        {
          return false;
        }

        if (!hasRecords)
        {
          file.header.firstTimestamp =
            segment.header.firstTimestamp;

          hasRecords = true;
        }

        file.header.lastTimestamp =
          segment.header.lastTimestamp;

        previousTimestamp =
          segment.header.lastTimestamp;
      }

      if (segment.header.recordCount >
          std::numeric_limits<uint64_t>::max() -
            totalRecords)
      {
        return false;
      }

      totalRecords += segment.header.recordCount;

      segment.archive.close();

      file.segments.push_back(
        std::move(segment));
    }

    file.header.recordCount = totalRecords;

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
  {
    for (auto& segment : file.segments)
      segment.archive.close();

    file.openSegment.reset();
  }

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

  return &m_files[fileIndex].header;
}

bool ArchiveReader::read(
  std::size_t fileIndex,
  ArchiveSample& sample)
{
  if (!m_open || fileIndex >= m_files.size())
    return false;

  return readRecord(
    fileIndex,
    m_files[fileIndex].nextRecord,
    sample);
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
  std::size_t segmentIndex = 0;
  uint64_t localIndex = 0;

  if (!locateRecord(
        fileIndex,
        recordIndex,
        segmentIndex,
        localIndex))
  {
    return false;
  }

  uint64_t position = 0;

  if (!recordPosition(
        fileIndex,
        segmentIndex,
        localIndex,
        position))
  {
    return false;
  }

  auto& archive =
    m_files[fileIndex].segments[segmentIndex].archive;

  if (!archive.seek(position))
    return false;

  SampleRecordHeader header;

  if (!archive.readObject(header))
    return false;

  ArchiveSample result;

  result.frameNumber = FrameNumber{header.frameNumber};
  result.timestamp = Timestamp{header.timestamp};
  result.wallTime = WallClockTime{header.wallTime};

  result.values.resize(
    m_files[fileIndex].header.channelCount);

  if (!archive.readArray(
        result.values.data(),
        result.values.size()))
  {
    return false;
  }

  sample = std::move(result);

  m_files[fileIndex].nextRecord = recordIndex + 1;

  return true;
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

  if (firstFrame.value > lastFrame.value ||
      maxRecords == 0)
  {
    return false;
  }

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

  while (recordIndex < header->recordCount &&
         samples.size() < maxRecords)
  {
    ArchiveSample sample;

    if (!readRecord(
          fileIndex,
          recordIndex,
          sample))
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

  const uint64_t count =
    std::min<uint64_t>(
      header->recordCount - firstRecord,
      maxRecords);

  samples.reserve(
    static_cast<std::size_t>(count));

  for (uint64_t i = 0; i < count; ++i)
  {
    ArchiveSample sample;

    if (!readRecord(
          fileIndex,
          firstRecord + i,
          sample))
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

bool ArchiveReader::readRecordHeader(
  std::size_t fileIndex,
  uint64_t recordIndex,
  SampleRecordHeader& header)
{
  std::size_t segmentIndex = 0;
  uint64_t localIndex = 0;

  if (!locateRecord(
        fileIndex,
        recordIndex,
        segmentIndex,
        localIndex))
  {
    return false;
  }

  uint64_t position = 0;

  if (!recordPosition(
        fileIndex,
        segmentIndex,
        localIndex,
        position))
  {
    return false;
  }

  auto& archive =
    m_files[fileIndex].segments[segmentIndex].archive;

  if (!archive.seek(position))
    return false;

  return archive.readObject(header);
}

bool ArchiveReader::ensureOpen(
  std::size_t fileIndex,
  std::size_t segmentIndex)
{
  if (!m_open || fileIndex >= m_files.size())
    return false;

  auto& file = m_files[fileIndex];

  if (segmentIndex >= file.segments.size())
    return false;

  // Нужный сегмент уже открыт.
  if (file.openSegment == segmentIndex)
  {
    return file.segments[segmentIndex].archive.isOpen();
  }

  // Закрываем предыдущий сегмент.
  if (file.openSegment)
  {
    file.segments[*file.openSegment].archive.close();
    file.openSegment.reset();
  }

  auto& segment = file.segments[segmentIndex];

  if (!segment.archive.open(
        segment.path,
        OpenMode::Read))
  {
    return false;
  }

  const auto& header = segment.archive.header();

  // Повторная проверка заголовка.
  if (!header.isValid() ||
      header.sampleFrequency != segment.header.sampleFrequency ||
      header.channelCount != segment.header.channelCount ||
      header.recordSize != segment.header.recordSize ||
      header.recordCount != segment.header.recordCount ||
      header.headerSize != segment.header.headerSize ||
      header.firstTimestamp != segment.header.firstTimestamp ||
      header.lastTimestamp != segment.header.lastTimestamp)
  {
    segment.archive.close();
    return false;
  }

  file.openSegment = segmentIndex;

  return true;
}

bool ArchiveReader::locateRecord(
  std::size_t fileIndex,
  uint64_t recordIndex,
  std::size_t& segmentIndex,
  uint64_t& localIndex)
{
  if (!m_open || fileIndex >= m_files.size())
    return false;

  const auto& file = m_files[fileIndex];

  if (recordIndex >= file.header.recordCount)
    return false;

  const auto it = std::upper_bound(
    file.segments.begin(),
    file.segments.end(),
    recordIndex,
    [](uint64_t index, const Segment& segment)
    {
      return index < segment.firstRecord;
    });

  if (it == file.segments.begin())
    return false;

  const auto selected = std::prev(it);

  segmentIndex = static_cast<std::size_t>(
    std::distance(file.segments.begin(), selected));

  localIndex = recordIndex - selected->firstRecord;

  return localIndex < selected->header.recordCount;
}

bool ArchiveReader::recordPosition(
  std::size_t fileIndex,
  std::size_t segmentIndex,
  uint64_t localIndex,
  uint64_t& position)
{
  if (!ensureOpen(fileIndex, segmentIndex))
    return false;

  auto& segment =
    m_files[fileIndex].segments[segmentIndex];

  const auto& header = segment.header;

  if (localIndex >= header.recordCount)
    return false;

  const uint64_t fileSize =
    segment.archive.fileSize();

  if (header.headerSize > fileSize)
    return false;

  const uint64_t available =
    fileSize - header.headerSize;

  if (header.recordSize == 0 ||
      header.recordSize > available)
  {
    return false;
  }

  if (localIndex >
      (available - header.recordSize) /
        header.recordSize)
  {
    return false;
  }

  position =
    static_cast<uint64_t>(header.headerSize) +
    localIndex *
      static_cast<uint64_t>(header.recordSize);

  return true;
}

}
