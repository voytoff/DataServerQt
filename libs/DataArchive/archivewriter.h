#ifndef ARCHIVEWRITER_H
#define ARCHIVEWRITER_H

#include <filesystem>
#include <span>

#include "archiveformat.h"
#include "archivefile.h"

namespace qds
{

class ArchiveWriter
{
public:
  bool open(
    const std::filesystem::path& file,
    const DataFileHeader& header);

  bool append(
    uint64_t timestamp,
    std::span<const float> values);

  bool flush();

  void close();

private:
  bool writeHeader();

private:
  ArchiveFile m_file;
  DataFileHeader m_header;
};

}

#endif // ARCHIVEWRITER_H
