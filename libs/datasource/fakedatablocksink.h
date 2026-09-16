#pragma once

#include "idatablocksink.h"

namespace qds
{

class FakeDataBlockSink : public IDataBlockSink
{
public:
  void push(
    ModuleId module,
    std::span<const double> values,
    std::size_t channelCount,
    std::size_t frameCount) override;

public:
  ModuleId m_module;
  std::span<const double> m_values;
  std::size_t m_channelCount;
  std::size_t m_frameCount;

};

}