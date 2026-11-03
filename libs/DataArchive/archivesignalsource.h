#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "archivereader.h"
#include "archivesignalpoint.h"

namespace qds
{

struct ArchiveSignalLocation
{
  std::size_t fileIndex = 0;
  std::size_t channelIndex = 0;
};

class ArchiveSignalSource
{
public:
  explicit ArchiveSignalSource(ArchiveReader& reader);

  [[nodiscard]]
  std::optional<ArchiveSignalLocation>
  findSignal(SignalId signalId) const;

  [[nodiscard]]
  bool readRange(
    SignalId signalId,
    Timestamp firstTimestamp,
    Timestamp lastTimestamp,
    std::size_t maxRecords,
    ArchiveSignalPoints& points);

private:
  ArchiveReader& m_reader;
};

}