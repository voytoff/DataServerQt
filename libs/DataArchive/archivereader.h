#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

#include "archivedescription.h"
#include "archivefile.h"
#include "datatypes.h"

namespace qds
{

struct ArchiveLocation
{
  std::size_t fileIndex = 0;
  std::size_t channelIndex = 0;
};

class ArchiveReader
{
public:

  bool open(
    const std::filesystem::path& directory);

  void close() noexcept;

  [[nodiscard]]
  bool isOpen() const noexcept;

  [[nodiscard]]
  const ArchiveDescription& description() const noexcept;

  [[nodiscard]]
  std::size_t fileCount() const noexcept;

  [[nodiscard]]
  const ArchiveFileDescription& fileDescription(
    std::size_t fileIndex) const;

  [[nodiscard]]
  const DataFileHeader* fileHeader(
    std::size_t fileIndex);

  bool read(
    std::size_t fileIndex,
    ArchiveSample& sample);

  bool readFrame(
    std::size_t fileIndex,
    FrameNumber frameNumber,
    ArchiveSample& sample);

  [[nodiscard]]
  bool readRecord(
    std::size_t fileIndex,
    uint64_t recordIndex,
    ArchiveSample& sample);

  [[nodiscard]]
  bool findRecord(
    std::size_t fileIndex,
    FrameNumber frameNumber,
    uint64_t& recordIndex);

  [[nodiscard]]
  bool findRecord(
    std::size_t fileIndex,
    Timestamp timestamp,
    uint64_t& recordIndex);

  [[nodiscard]]
  bool readRange(
    std::size_t fileIndex,
    FrameNumber firstFrame,
    FrameNumber lastFrame,
    std::vector<ArchiveSample>& samples);

  [[nodiscard]]
  bool readRange(
    std::size_t fileIndex,
    FrameNumber firstFrame,
    FrameNumber lastFrame,
    std::size_t maxRecords,
    std::vector<ArchiveSample>& samples);

  [[nodiscard]]
  bool readRecords(
    std::size_t fileIndex,
    uint64_t firstRecord,
    std::size_t maxRecords,
    std::vector<ArchiveSample>& samples);

  [[nodiscard]]
  bool timeRange(
    Timestamp& firstTimestamp,
    Timestamp& lastTimestamp);

  [[nodiscard]]
  bool timeReference(
    Timestamp& timestamp,
    WallClockTime& wallTime);

  [[nodiscard]]
  std::optional<ArchiveLocation> findSignal(
    SignalId signalId) const;

private:
  bool ensureOpen(
    std::size_t fileIndex);

  bool readRecordHeader(
    std::size_t fileIndex,
    uint64_t recordIndex,
    SampleRecordHeader& header);

  bool recordPosition(
    std::size_t fileIndex,
    uint64_t recordIndex,
    uint64_t& position);

private:

  struct File
  {
    ArchiveFileDescription description;
    std::filesystem::path path;
    ArchiveFile archive;
  };

  std::filesystem::path m_directory;
  ArchiveDescription m_description;
  std::vector<File> m_files;

  bool m_open = false;
};

}