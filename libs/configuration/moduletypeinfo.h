#pragma once

#include "moduletype.h"

#include <cstdint>

namespace qds
{

struct ModuleTypeInfo
{
  ModuleType type = ModuleType::Unknown;
  uint32_t channelCount = 0;
  uint32_t maxChannels = 0;
};

[[nodiscard]]
const ModuleTypeInfo* moduleTypeInfo(
  ModuleType type) noexcept;

}