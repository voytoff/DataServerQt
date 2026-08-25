#include "archivewriter.h"

namespace qds
{

bool ArchiveWriter::open(
  const std::filesystem::path& path,
  const DataFileHeader& header)
{
  return m_file.create(path, header);
}


void ArchiveWriter::close()
{
  m_file.close();
}


bool ArchiveWriter::isOpen() const noexcept
{
  return m_file.isOpen();
}


bool ArchiveWriter::write(
  Timestamp timestamp,
  FrameNumber frameNumber,
  WallClockTime wallTime,
  std::span<const float> values)
{
  if (!m_file.isOpen())
    return false;

  if (values.size() != m_file.header().channelCount ||
      values.empty())
    return false;

  SampleRecordHeader record {
    .timestamp = timestamp.value,
    .frameNumber = frameNumber.value,
    .wallTime = wallTime.unixMicroseconds
  };


  if (!m_file.writeObject(record))
    return false;


  if (!m_file.writeArray(
        values.data(),
        values.size()))
    return false;

  if (m_file.header().recordCount == 0)
    m_file.setFirstTimestamp(timestamp.value);

  m_file.setLastTimestamp(timestamp.value);
  m_file.recordWritten();


  return true;
}

bool ArchiveWriter::flush()
{
  return m_file.flush();
}

const DataFileHeader &qds::ArchiveWriter::header() const
{
  return m_file.header();
}


uint64_t ArchiveWriter::recordCount() const
{
  return header().recordCount;
}

uint64_t ArchiveWriter::fileSize()
{
  return m_file.fileSize();
}

}