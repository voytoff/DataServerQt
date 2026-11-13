
#pragma once

#include <filesystem>

#include "archivedescription.h"

namespace qds
{

class ArchiveDescriptionReader
{
public:
  bool read(
    const std::filesystem::path& path,
    ArchiveDescription& description) const;
};

}
