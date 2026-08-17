#pragma once

#include <span>
#include <vector>

#include "calculationstep.h"

namespace qds
{

class CalculationPlan
{
public:
  void clear() noexcept;

  void add(CalculationStep step);

  void reserve(std::size_t size);

  [[nodiscard]]
  std::size_t size() const noexcept;

  [[nodiscard]]
  bool empty() const noexcept;

  [[nodiscard]]
  const CalculationStep& at(
    std::size_t index) const;

  [[nodiscard]]
  std::span<const CalculationStep> steps() const noexcept;

private:
  std::vector<CalculationStep> m_steps;
};

}