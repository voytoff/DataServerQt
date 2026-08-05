#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include "calculationstep.h"

namespace qds
{

class CalculationPlan
{
public:
  void clear() noexcept;

  void addStep(CalculationStep step);

  [[nodiscard]]
  std::span<const CalculationStep> steps() const noexcept;

  [[nodiscard]]
  bool empty() const noexcept;

  [[nodiscard]]
  uint32_t size() const noexcept;

private:
  std::vector<CalculationStep> m_steps;

};

}