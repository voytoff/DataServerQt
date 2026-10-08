#include "moduletypeinfo.h"

#include <array>

namespace qds
{

namespace
{

constexpr std::array moduleTypes{
  ModuleTypeInfo{ModuleType::LTR11,    32, 32},
  ModuleTypeInfo{ModuleType::LTR114,   16, 16},
  ModuleTypeInfo{ModuleType::LTR41,    16, 16},
  ModuleTypeInfo{ModuleType::LTR42,    16, 16},
  ModuleTypeInfo{ModuleType::LTR43DI4,  4, 32},
  ModuleTypeInfo{ModuleType::LTR22,     4,  4},
  ModuleTypeInfo{ModuleType::LTR210,    2,  2},
  ModuleTypeInfo{ModuleType::LTR24,     4,  4}
};

}

const ModuleTypeInfo* moduleTypeInfo(
  ModuleType type) noexcept
{
  for (const auto& info : moduleTypes)
  {
    if (info.type == type)
      return &info;
  }

  return nullptr;
}

}