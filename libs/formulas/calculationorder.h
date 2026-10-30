#pragma once

#include "datatypes.h"

#include <span>
#include <string>
#include <unordered_map>
#include <vector>
#include <strongidhash.h>

namespace qds
{

class SystemConfiguration;

class CalculationOrder
{
public:

  bool build(
    const SystemConfiguration& configuration);

  void clear();

  [[nodiscard]]
  std::span<const SignalId> order() const noexcept;

  [[nodiscard]]
  const std::string& lastError() const noexcept;

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
    VisitStates& states,
    std::vector<SignalId>& path);

  bool fail(std::string message);

  void setCycleError(
    SignalId id,
    const SystemConfiguration& configuration,
    const std::vector<SignalId>& path);

private:

  std::vector<SignalId> m_order;
  std::string m_lastError;
};

}