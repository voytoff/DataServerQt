#include "calculationplan.h"

namespace qds
{

void CalculationPlan::clear() noexcept
{
  m_order.clear();
}

void CalculationPlan::setOrder(std::vector<SignalId> order)
{
  m_order = order;
}

bool CalculationPlan::isEmpty() const noexcept
{
  return m_order.size() == 0;
}

std::span<const SignalId> CalculationPlan::order() const noexcept
{
  return m_order;
}

}