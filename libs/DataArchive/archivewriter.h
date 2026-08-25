#pragma once

#include "archivefile.h"
#include <span>

namespace qds
{

class ArchiveWriter
{
public:
  ArchiveWriter() = default;

  bool open(
    const std::filesystem::path& path,
    const DataFileHeader& header);

  void close();

  bool isOpen() const noexcept;

  bool write(
    Timestamp timestamp,
    FrameNumber frameNumber,
    WallClockTime wallTime,
    std::span<const float> values);

  bool flush();

  const DataFileHeader& header() const;

  uint64_t recordCount() const;

  uint64_t fileSize();

private:
  ArchiveFile m_file;

};

}