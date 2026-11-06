#pragma once

#include "archivereader.h"
#include "archivesignalpoint.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <atomic>
#include <vector>

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

  [[nodiscard]]
  bool readPlot(
    SignalId signalId,
    Timestamp firstTimestamp,
    Timestamp lastTimestamp,
    std::size_t maxPoints,
    ArchiveSignalPoints& points,
    const std::atomic_bool* cancel = nullptr);

  [[nodiscard]]
  bool readPlotSegments(
    SignalId signalId,
    Timestamp firstTimestamp,
    Timestamp lastTimestamp,
    std::size_t maxPoints,
    ArchiveSignalSegments& segments,
    const std::atomic_bool* cancel = nullptr);

  [[nodiscard]] bool timeRange(
    SignalId signalId,
    Timestamp& firstTimestamp,
    Timestamp& lastTimestamp) const;

private:
  template<typename Callback>
  bool forEachSample(
    std::size_t fileIndex,
    uint64_t firstRecord,
    Timestamp lastTimestamp,
    const std::atomic_bool* cancel,
    bool includeNext,
    Callback&& callback)
  {
    constexpr std::size_t blockSize = 4096;

    std::vector<ArchiveSample> samples;
    uint64_t recordIndex = firstRecord;

    while (true)
    {
      if (cancel && cancel->load(std::memory_order_relaxed))
        return false;

      if (!m_reader.readRecords(
            fileIndex,
            recordIndex,
            blockSize,
            samples))
      {
        return false;
      }

      if (samples.empty())
        break;

      for (const auto& sample : samples)
      {
        if (cancel && cancel->load(std::memory_order_relaxed))
          return false;

        if (sample.timestamp > lastTimestamp)
        {
          if (includeNext && !callback(sample))
            return false;

          return true;
        }

        if (!callback(sample))
          return false;
      }

      recordIndex += samples.size();
    }

    return !(cancel && cancel->load(std::memory_order_relaxed));
  }

private:
  ArchiveReader& m_reader;
};

}