#pragma once

#include <filesystem>

#include "archivedescription.h"

namespace qds
{

class ArchiveDescriptionWriter
{
public:

  bool write(
    const std::filesystem::path& path,
    const ArchiveDescription& description) const;
};

}