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

QString moduleTypeToString(
  ModuleType type)
{
  switch (type)
  {
  case ModuleType::LTR11:    return "LTR11";
  case ModuleType::LTR114:   return "LTR114";
  case ModuleType::LTR212:   return "LTR212";
  case ModuleType::LTR27:    return "LTR27";
  case ModuleType::LTR41:    return "LTR41";
  case ModuleType::LTR42:    return "LTR42";
  case ModuleType::LTR43DI4: return "LTR43DI4";
  case ModuleType::LTR51:    return "LTR51";
  case ModuleType::LTR22:    return "LTR22";
  case ModuleType::LTR210:   return "LTR210";
  case ModuleType::LTR24:    return "LTR24";
  case ModuleType::LTR34:    return "LTR34";

  case ModuleType::Test:     return "Test";
  case ModuleType::Fake:     return "Fake";
  case ModuleType::Failing:  return "Failing";
  case ModuleType::FailOnce: return "FailOnce";

  default:
    return "Unknown";
  }
}

}