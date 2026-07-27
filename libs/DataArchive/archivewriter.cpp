#include "archivewriter.h"

namespace qds
{



bool ArchiveWriter::open(const std::filesystem::path &path, const DataFileHeader &header)
{
  return true;
}

bool ArchiveWriter::write(uint64_t timestamp, std::span<const float> values)
{
  return true;
}

void ArchiveWriter::close()
{

}

}
