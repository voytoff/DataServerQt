#ifndef IMODULEGENERATOR_H
#define IMODULEGENERATOR_H

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

#endif // IMODULEGENERATOR_H
