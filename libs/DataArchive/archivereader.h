#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

#include "archivedescription.h"
#include "archivefile.h"
#include "datatypes.h"

namespace qds
{

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

private:

  struct File
  {
    ArchiveFileDescription description;
    std::filesystem::path path;
    ArchiveFile archive;
  };

  bool ensureOpen(
    std::size_t fileIndex);

  std::filesystem::path m_directory;

  ArchiveDescription m_description;

  std::vector<File> m_files;

  bool m_open = false;
};

}