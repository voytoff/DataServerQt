#include "calculationplan.h"

namespace qds
{

void CalculationPlan::clear() noexcept
{
  m_steps.clear();
}

void CalculationPlan::addStep(CalculationStep step)
{
  m_steps.push_back(step);
}

std::span<const CalculationStep> CalculationPlan::steps() const noexcept
{
  return m_steps;
}

bool CalculationPlan::empty() const noexcept
{
  return m_steps.empty();
}

uint32_t CalculationPlan::size() const noexcept
{
  return m_steps.size();
}

}