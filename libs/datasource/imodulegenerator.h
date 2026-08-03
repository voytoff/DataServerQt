#pragma once

#include "datatypes.h"
#include <span>

namespace qds
{

enum class GeneratorMode
{
  Constant,
  Counter
};

/** интерфейс генератора + GeneratorMode */
class IModuleGenerator
{
public:
  virtual ~IModuleGenerator() = default;
  virtual void generate(ModuleId module, std::span<float> values) = 0;
};

}