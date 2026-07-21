#include "countergenerator.h"
#include <qtpreprocessorsupport.h>

namespace qds
{

void CounterGenerator::generate(ModuleId module, std::span<float> values)
{
  Q_UNUSED(module);
  for (float& value : values)
    value = m_counter++;
}

}