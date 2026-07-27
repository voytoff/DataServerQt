#ifndef ARCHIVEREADER_H
#define ARCHIVEREADER_H

#include <cstdint>
#include <span>

#include "archiveformat.h"
#include "archivefile.h"

namespace qds
{

class ArchiveReader
{
public:

  bool open();

  const DataFileHeader& header() const;

  bool read(
    uint64_t& timestamp,
    std::span<float> values);

private:

  ArchiveFile m_file;

  DataFileHeader m_header;
};

}


#endif // ARCHIVEREADER_H
