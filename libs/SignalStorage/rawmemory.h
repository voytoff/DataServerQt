#ifndef RAWMEMORY_H
#define RAWMEMORY_H

#include <cstdint>
#include <span>
#include <vector>

namespace qds
{

class SignalMemoryLayout;

class RawMemory
{
public:

  void initialize(
    const SignalMemoryLayout& layout) noexcept;

  [[nodiscard]]
  double value(uint32_t index) const;

  void setValue(
    uint32_t index,
    double value);

  void setValues(
    uint32_t firstIndex,
    std::span<const double> values);

  [[nodiscard]]
  std::span<const double> values() const noexcept;

  void snapshot(
    std::span<double> destination) const;

private:

  std::vector<double> m_values;
};

}

#endif // RAWMEMORY_H
