#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace qds
{

class SignalMemoryLayout;

class SignalStorage
{
public:
  void initialize(const SignalMemoryLayout& layout) ;

  [[nodiscard]]
  double rawValue(uint32_t index) const;
  [[nodiscard]]
  double calculatedValue(uint32_t index) const;

  void setRawValue(uint32_t index, double value);
  void setCalculatedValue(uint32_t index, double value);

  [[nodiscard]]
  std::span<const double> rawValues() const noexcept;
  [[nodiscard]]
  std::span<const double> calculatedValues() const noexcept;

  std::span<double> rawValues() noexcept;
  std::span<double> calculatedValues() noexcept;

private:
  std::vector<double> m_raw;
  std::vector<double> m_calculated;

};

}
