#pragma once

#include <cstdint>
#include <filesystem>
#include <span>

#include "archiveformat.h"
#include "archivefile.h"

namespace qds
{

class ArchiveReader
{
public:
  bool open(const std::filesystem::path& file);

  bool readNext(
    uint64_t& timestamp,
    std::span<float> values);

  void close();

  const DataFileHeader& header() const;

private:
  ArchiveFile m_file;
  DataFileHeader m_header;
};

}