#ifndef ARCHIVEWRITER_H
#define ARCHIVEWRITER_H

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
    uint64_t timestamp,
    std::span<const float> values);

  bool flush();

  ArchiveFile& file() noexcept;
  const ArchiveFile& file() const noexcept;

private:
  ArchiveFile m_file;

};

}

#endif