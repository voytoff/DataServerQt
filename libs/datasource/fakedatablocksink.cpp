#include "fakedatablocksink.h"

namespace qds
{

void FakeDataBlockSink::push(
  ModuleId module,
  std::span<const double> values,
  std::size_t channelCount,
  std::size_t frameCount)
{
  m_module = module;
  m_values = values;
  m_channelCount = channelCount;
  m_frameCount = frameCount;
}

}
