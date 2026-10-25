#include "countergenerator.h"

namespace qds
{

void CounterGenerator::generate(ModuleId, std::span<float> values)
{
  for (float& value : values)
    value = m_counter++;
}

}