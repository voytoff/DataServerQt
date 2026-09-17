#pragma once

#include "idatablocksink.h"

#include <vector>

namespace qds
{

class FakeDataBlockSink final : public IDataBlockSink
{
public:
  void push(
    ModuleId module,
    uint64_t firstFrameIndex,
    std::span<const double> values,
    std::size_t channelCount,
    std::size_t frameCount,
    double frameRate) override;

public:
  ModuleId m_module;
  std::vector<double> m_values;
  std::size_t m_channelCount = 0;
  std::size_t m_frameCount = 0;
  double m_frameRate = 0.0;
};

}