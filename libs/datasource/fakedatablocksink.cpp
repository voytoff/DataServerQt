#include "fakedatablocksink.h"

namespace qds
{

void FakeDataBlockSink::push(
  ModuleId module,
  uint64_t firstFrameIndex,
  std::span<const double> values,
  std::size_t channelCount,
  std::size_t frameCount,
  double frameRate)
{
  m_module = module;

  m_values.assign(
    values.begin(),
    values.end());

  m_channelCount = channelCount;
  m_frameCount = frameCount;
  m_frameRate = frameRate;
}

}
