#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "archivedescription.h"
#include "archivewriter.h"
#include "iarchivewriter.h"
#include "ilogger.h"

namespace qds
{

class ArchiveManager : public IArchiveWriter
{
public:
  explicit ArchiveManager(
    ILogger& logger);

  bool initialize(
    const std::filesystem::path& directory,
    const ArchiveDescription& description,
    const SignalMemoryLayout& layout,
    uint32_t segmentDuration);

  bool write(const Frame& frame) override;

  void close();

private:
  struct ArchiveSignalBinding
  {
    SignalMemoryArea area;
    uint32_t memoryIndex;
  };

  struct ArchiveTarget
  {
    std::string fileName;

    uint32_t frequency = 0;
    uint32_t periodFrames = 0;

    DataFileHeader header;
    ArchiveWriter writer;

    std::vector<ArchiveSignalBinding> signalBindings;
    std::vector<float> values;
  };

  bool openSegment(uint64_t index);

  ILogger& m_logger;

  std::filesystem::path m_directory;

  std::vector<ArchiveTarget> m_targets;

  uint64_t m_segmentFrameCount = 0;
  uint64_t m_firstFrameNumber = 0;
  std::optional<uint64_t> m_currentSegment;

  bool m_initialized = false;
  std::optional<uint64_t> m_previousFrameNumber;
};

}