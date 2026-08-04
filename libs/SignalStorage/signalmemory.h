#ifndef SIGNALMEMORY_H
#define SIGNALMEMORY_H

#include <cstdint>
#include <span>
#include <vector>

namespace qds
{

class SignalMemory
{
public:
  // initialize() полностью пересоздает память.
  // Старые значения уничтожаются.
  void initialize(uint32_t count) noexcept;

  [[nodiscard]]
  double value(uint32_t index) const;

  void setValue(uint32_t index, double value);

  void setValues(uint32_t first, std::span<const double> values);

  [[nodiscard]]
  std::span<const double> values() const noexcept;
  [[nodiscard]]
  std::span<double> values() noexcept;

  void snapshot(std::span<double> destination) const;

  void restore(std::span<const double> source);

  bool equals(std::span<const double> snapshot) const;

  void clear() noexcept;

private:
  std::vector<double> m_values;

};

using RawMemory = SignalMemory;
using CalculatedMemory = SignalMemory;

}

#endif // SIGNALMEMORY_H
