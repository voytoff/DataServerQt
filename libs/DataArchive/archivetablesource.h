#pragma once

#include "archivetableblock.h"
#include "archivereader.h"
#include "datatypes.h"

#include <atomic>
#include <cstdint>
#include <vector>

namespace qds
{

class ArchiveTableSource
{
public:
  explicit ArchiveTableSource(
    ArchiveReader& reader);

  [[nodiscard]]
  bool readBlock(
    Timestamp experimentStart,
    uint32_t frequency,
    uint64_t firstRow,
    uint32_t rowCount,
    const std::vector<SignalId>& channels,
    ArchiveTableBlock& block,
    const std::atomic_bool* cancel = nullptr);

private:
  ArchiveReader& m_reader;
};

}