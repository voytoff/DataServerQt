#include "calculationplan.h"

#include <cassert>

namespace qds
{

void CalculationPlan::clear() noexcept
{
  m_steps.clear();
}

void CalculationPlan::add(
  CalculationStep step)
{
  m_steps.push_back(step);
}

void CalculationPlan::reserve(std::size_t size)
{
  m_steps.reserve(size);
}

std::size_t CalculationPlan::size() const noexcept
{
  return m_steps.size();
}

bool CalculationPlan::empty() const noexcept
{
  return m_steps.empty();
}

const CalculationStep& CalculationPlan::at(
  std::size_t index) const
{
  assert(index < m_steps.size());

  return m_steps[index];
}

std::span<const CalculationStep>
CalculationPlan::steps() const noexcept
{
  return m_steps;
}

}