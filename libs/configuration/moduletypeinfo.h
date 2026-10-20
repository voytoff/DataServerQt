#pragma once

#include "moduletype.h"

#include <cstdint>
#include <QString>

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

QString moduleTypeToString(
  ModuleType type);

}