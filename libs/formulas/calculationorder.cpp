#include "calculationorder.h"
#include "systemconfiguration.h"

namespace qds
{

bool CalculationOrder::build(
  const SystemConfiguration& configuration)
{
  m_order.clear();

  VisitStates states;

  for (const auto& definition :
       configuration.signalDefinitions())
  {
    if (definition.kind != SignalKind::Calculated)
      continue;

    states.emplace(
      definition.id,
      VisitState::NotVisited);
  }

  for (const auto& definition :
       configuration.signalDefinitions())
  {
    if (definition.kind != SignalKind::Calculated)
      continue;

    if (!visit(
          definition.id,
          configuration,
          states))
    {
      m_order.clear();
      return false;
    }
  }

  return true;
}

void CalculationOrder::clear()
{
  m_order.clear();
}

bool CalculationOrder::visit(
  SignalId id,
  const SystemConfiguration& configuration,
  VisitStates& states)
{
  auto stateIt = states.find(id);

  if (stateIt == states.end())
    return false;

  switch (stateIt->second)
  {
  case VisitState::Visited:
    return true;

  case VisitState::Visiting:
    // Обнаружили цикл.
    return false;

  case VisitState::NotVisited:
    break;
  }

  stateIt->second = VisitState::Visiting;

  const auto* definition =
    configuration.findSignalDefinition(id);

  if (definition == nullptr)
    return false;

  for (const SignalId& dependency :
       definition->dependencies)
  {
    const auto* dependencyDefinition =
      configuration.findSignalDefinition(
        dependency);

    if (dependencyDefinition == nullptr)
      return false;

    if (dependencyDefinition->kind ==
        SignalKind::Raw)
    {
      continue;
    }

    if (dependencyDefinition->kind !=
        SignalKind::Calculated)
    {
      return false;
    }

    if (!visit(
          dependency,
          configuration,
          states))
    {
      return false;
    }
  }

  stateIt->second = VisitState::Visited;

  m_order.push_back(id);

  return true;
}

std::span<const SignalId>
CalculationOrder::order() const noexcept
{
  return m_order;
}

}