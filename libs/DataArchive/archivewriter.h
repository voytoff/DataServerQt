#ifndef ARCHIVEWRITER_H
#define ARCHIVEWRITER_H

#include "datafileheader.h"
#include <filesystem>
#include <span>

namespace qds
{

class ArchiveWriter
{
public:

  bool open(
    const DataFileHeader& header,
    const std::filesystem::path& file);

  void close();

  bool write(
    uint64_t timestamp,
    std::span<const float> values);

private:

  std::ofstream *m_file;
  DataFileHeader m_header{};
};

}

#endif // ARCHIVEWRITER_H
