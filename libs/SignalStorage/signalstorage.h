#pragma once

#include <cstdint>
#include <vector>

namespace qds
{

class SignalStorage
{
public:

  void initialize(uint32_t signalCount);

  double value(uint32_t index) const;

  void setValue(uint32_t index, double value);

  uint32_t size() const;

private:

  std::vector<double> m_values;
};

}
