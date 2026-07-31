#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include "signalmemorylayout.h"

namespace qds
{

class SignalStorage
{
public:
  void initialize(const SignalMemoryLayout& layout);

  [[nodiscard]]
  float rawValue(uint32_t index) const;
  [[nodiscard]]
  float calculatedValue(uint32_t index) const;

  void setRawValue(uint32_t index, float value);
  void setCalculatedValue(uint32_t index, float value);

  std::span<float> rawValues();
  std::span<float> calculatedValues();

private:
  std::vector<float> m_raw;
  std::vector<float> m_calculated;

};

}
