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

  [[nodiscard]]
  std::span<const CalculationStep> steps() const noexcept;

  [[nodiscard]]
  bool empty() const noexcept;

  [[nodiscard]]
  std::size_t size() const noexcept;

private:
  void addStep(CalculationStep&& step);

  void reserve(std::size_t size);

private:
  std::vector<CalculationStep> m_steps;

  friend class CalculationCompiler;
};

}