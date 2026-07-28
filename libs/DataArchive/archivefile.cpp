#include "archivefile.h"

namespace qds
{

ArchiveFile::~ArchiveFile()
{
  close();
}

void ArchiveFile::close()
{
  if (m_stream.is_open())
    m_stream.close();

  m_stream.clear();
  m_mode = OpenMode::Closed;
}

bool ArchiveFile::create(
  const std::filesystem::path& path,
  const DataFileHeader& header)
{
  if (!header.isValid())
    return false;

  reset();

  m_stream.open(
    path,
    std::ios::binary |
      std::ios::out |
      std::ios::trunc);

  if (!m_stream)
    return false;

  m_path = path;
  m_mode = OpenMode::Write;
  m_header = header;

  if (!writeHeader())
  {
    reset();
    return false;
  }

  if (!flush())
  {
    reset();
    return false;
  }

  return true;
}

bool ArchiveFile::open(const std::filesystem::path& path, OpenMode mode)
{
  reset();

  std::ios::openmode ios = std::ios::binary;

  switch(mode)
  {
  case OpenMode::Read:
    ios |= std::ios::in;
    break;

  case OpenMode::Write:
    ios |= std::ios::out;
    break;

  case OpenMode::ReadWrite:
    ios |= std::ios::in | std::ios::out;
    break;

  default:
    return false;
  }

  m_stream.open(
    path,
    ios);

  if (!m_stream)
    return false;

  m_path = path;
  m_mode = mode;

  if (!readHeader())
  {
    reset();
    return false;
  }

  if (!m_header.isValid())
  {
    reset();
    return false;
  }

  return true;
}

bool ArchiveFile::isOpen() const noexcept
{
  return m_mode != OpenMode::Closed;
}

bool ArchiveFile::isWritable() const noexcept
{
  return m_mode == OpenMode::Write
         || m_mode == OpenMode::ReadWrite;
}

bool ArchiveFile::isReadable() const noexcept
{
  return m_mode == OpenMode::Read
         || m_mode == OpenMode::ReadWrite;
}

const std::filesystem::path &ArchiveFile::path() const noexcept
{
  return m_path;
}

const DataFileHeader &ArchiveFile::header() const noexcept
{
  return m_header;
}

bool ArchiveFile::saveHeader()
{
  if (!isWritable())
    return false;

  const uint64_t pos = position();

  if (!writeHeader())
    return false;

  if (!seek(pos))
    return false;

  return true;
}

OpenMode ArchiveFile::mode() const noexcept
{
  return m_mode;
}

bool ArchiveFile::write(std::span<const std::byte> data)
{
  if (!isWritable())
    return false;

  if (data.empty())
    return true;

  m_stream.write(
    reinterpret_cast<const char*>(data.data()),
    static_cast<std::streamsize>(data.size()));

  return bool(m_stream);
}

bool ArchiveFile::read(std::span<std::byte> data)
{
  if (!isReadable())
    return false;

  if (data.empty())
    return true;

  m_stream.read(
    reinterpret_cast<char*>(data.data()),
    static_cast<std::streamsize>(data.size()));

  return bool(m_stream);
}

uint64_t ArchiveFile::position()
{
  if (!isOpen())
    return 0;

  if (isWritable())
  {
    auto pos = m_stream.tellp();

    if (pos >= 0)
      return static_cast<uint64_t>(pos);
  }

  if (isReadable())
  {
    auto pos = m_stream.tellg();

    if (pos >= 0)
      return static_cast<uint64_t>(pos);
  }

  return 0;
}

bool ArchiveFile::seek(uint64_t pos)
{
  if (!isOpen())
    return false;

  m_stream.clear();   // сброс eof/fail после предыдущих операций

  const auto offset =
    static_cast<std::streamoff>(pos);

  if (isReadable())
  {
    m_stream.seekg(offset);
    return bool(m_stream);
  }

  if (isWritable())
  {
    m_stream.seekp(offset);
    return bool(m_stream);
  }

  return true;
}

uint64_t ArchiveFile::fileSize()
{
  if (!isOpen())
    return 0;

  const auto old = position();

  uint64_t result = 0;

  if (isReadable())
  {
    m_stream.seekg(0, std::ios::end);

    auto pos = m_stream.tellg();

    if (pos >= 0)
      result = static_cast<uint64_t>(pos);

    m_stream.seekg(
      static_cast<std::streamoff>(old));
  }
  else
  {
    m_stream.seekp(0, std::ios::end);

    auto pos = m_stream.tellp();

    if (pos >= 0)
      result = static_cast<uint64_t>(pos);

    m_stream.seekp(
      static_cast<std::streamoff>(old));
  }

  return result;
}

bool ArchiveFile::eof() const
{
  return isReadable() && m_stream.eof();
}

bool ArchiveFile::fail() const noexcept
{
  return m_stream.fail();
}

bool ArchiveFile::bad() const noexcept
{
  return m_stream.bad();
}

bool ArchiveFile::good() const noexcept
{
  return m_stream.good();
}

bool ArchiveFile::flush()
{
  if (!isWritable())
    return false;

  m_stream.flush();

  return bool(m_stream);
}

void ArchiveFile::reset()
{
  close();
  m_header = {};
  m_path.clear();
}

ArchiveFile::operator bool() const noexcept
{
  return isOpen();
}

void ArchiveFile::setFirstTimestamp(uint64_t ts) noexcept
{
  m_header.firstTimestamp = ts;
}

void ArchiveFile::setLastTimestamp(uint64_t ts) noexcept
{
  m_header.lastTimestamp = ts;
}

bool ArchiveFile::writeHeader() noexcept
{
  if (!isWritable())
    return false;

  m_stream.seekp(0);

  if (!m_stream)
    return false;

  m_stream.write(
    reinterpret_cast<const char*>(&m_header),
    sizeof(DataFileHeader));

  return bool(m_stream);
}

bool ArchiveFile::readHeader() noexcept
{
  if (!isReadable())
    return false;

  m_stream.seekg(0);

  if (!m_stream)
    return false;

  m_stream.read(
    reinterpret_cast<char*>(&m_header),
    sizeof(DataFileHeader));

  return bool(m_stream);
}

}
