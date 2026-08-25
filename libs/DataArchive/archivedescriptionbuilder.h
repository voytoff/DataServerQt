#pragma once

#include "archivedescription.h"
#include "systemconfiguration.h"

namespace qds
{

class ArchiveDescriptionBuilder
{
public:
  bool build(
    const SystemConfiguration& configuration,
    ArchiveDescription& description) const;
};

}