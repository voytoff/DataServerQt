#include "archivewriter.h"

namespace qds
{

bool ArchiveWriter::open(const DataFileHeader &header, const std::filesystem::path &file)
{
  auto recordSize =
    sizeof(uint64_t) +
    m_header.channelCount * sizeof(float);
}

bool ArchiveWriter::write(uint64_t timestamp, std::span<const float> values)
{
  if (values.size() != m_header.channelCount)
    return false;

}

}
