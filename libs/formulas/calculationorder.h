#pragma once

#include <span>
#include <unordered_map>
#include <vector>
#include <strongidhash.h>

#include "signaldefinition.h"

namespace qds
{

class SystemConfiguration;

class CalculationOrder
{
public:

  bool build(
    const SystemConfiguration& configuration);

  [[nodiscard]]
  std::span<const SignalId> order() const noexcept;

private:

  enum class VisitState
  {
    NotVisited,
    Visiting,
    Visited
  };

  using VisitStates =
    std::unordered_map<SignalId, VisitState>;

  bool visit(
    SignalId id,
    const SystemConfiguration& configuration,
    VisitStates& states);

private:

  std::vector<SignalId> m_order;
};

}