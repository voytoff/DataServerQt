#pragma once

#include "archivedescription.h"
#include "archivewriter.h"
#include "iarchivewriter.h"

namespace qds
{

class ArchiveManager : public IArchiveWriter
{
public:

  bool initialize(
    const std::filesystem::path& directory,
    const ArchiveDescription& description,
    const SignalMemoryLayout& layout);

  bool write(
    const Frame& frame) override;

  void close();

  [[nodiscard]]
  bool isInitialized() const noexcept;

private:

  struct ArchiveSignalBinding
  {
    SignalMemoryArea area;
    // signal.index — индекс в архиве, layout.reference(signal.id).index — индекс в Frame.
    uint32_t memoryIndex;
  };

  struct ArchiveTarget
  {
    uint32_t frequency = 0;
    uint32_t periodFrames = 0;

    ArchiveWriter writer;

    std::vector<ArchiveSignalBinding> signalBindings;
    std::vector<float> values;
  };

  std::vector<ArchiveTarget> m_targets;
};

}