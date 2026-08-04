#pragma once

#include "signaldefinition.h"
#include <span>
#include <vector>

namespace qds
{

class CalculationPlan
{
public:

  void clear() noexcept;

  void setOrder(std::vector<SignalId> order);

  [[nodiscard]]
  bool isEmpty() const noexcept;

  [[nodiscard]]
  size_t size() const noexcept;

  [[nodiscard]]
  std::span<const SignalId> order() const noexcept;

private:

  std::vector<SignalId> m_order;
};

}