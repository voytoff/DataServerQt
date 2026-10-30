#include "calculationorder.h"
#include "systemconfiguration.h"

#include <algorithm>
#include <utility>

namespace qds
{

bool CalculationOrder::build(
  const SystemConfiguration& configuration)
{
  clear();

  VisitStates states;
  std::vector<SignalId> path;

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
          states,
          path))
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
  m_lastError.clear();
}

bool CalculationOrder::fail(
  std::string message)
{
  if (m_lastError.empty())
    m_lastError = std::move(message);

  return false;
}

const std::string&
CalculationOrder::lastError() const noexcept
{
  return m_lastError;
}

bool CalculationOrder::visit(
  SignalId id,
  const SystemConfiguration& configuration,
  VisitStates& states,
  std::vector<SignalId>& path)
{
  auto stateIt = states.find(id);

  if (stateIt == states.end())
  {
    return fail(
      "Сигнал отсутствует в графе вычислений (SignalId: " +
      std::to_string(id.value) + ")");
  }

  switch (stateIt->second)
  {
  case VisitState::Visited:
    return true;

  case VisitState::Visiting:
    setCycleError(
      id,
      configuration,
      path);

    return false;

  case VisitState::NotVisited:
    break;
  }

  stateIt->second = VisitState::Visiting;
  path.push_back(id);

  const auto* definition =
    configuration.findSignalDefinition(id);

  if (definition == nullptr)
  {
    return fail(
      "Не найден вычисляемый сигнал (SignalId: " +
      std::to_string(id.value) + ")");
  }

  for (const SignalId& dependency :
       definition->dependencies)
  {
    const auto* dependencyDefinition =
      configuration.findSignalDefinition(
        dependency);

    if (dependencyDefinition == nullptr)
    {
      return fail(
        "Сигнал \"" + definition->name +
        "\" ссылается на отсутствующий сигнал (SignalId: " +
        std::to_string(dependency.value) + ")");
    }

    if (dependencyDefinition->kind ==
        SignalKind::Raw)
    {
      continue;
    }

    if (dependencyDefinition->kind !=
        SignalKind::Calculated)
    {
      return fail(
        "Недопустимый вид зависимости сигнала \"" +
        definition->name + "\"");
    }

    if (!visit(
          dependency,
          configuration,
          states,
          path))
    {
      return false;
    }
  }

  path.pop_back();

  stateIt->second = VisitState::Visited;

  m_order.push_back(id);

  return true;
}

void CalculationOrder::setCycleError(
  SignalId id,
  const SystemConfiguration& configuration,
  const std::vector<SignalId>& path)
{
  std::string message =
    "Обнаружена циклическая зависимость сигналов: ";

  const auto it =
    std::find(path.begin(), path.end(), id);

  if (it == path.end())
  {
    fail(
      "Обнаружена циклическая зависимость (SignalId: " +
      std::to_string(id.value) + ")");

    return;
  }

  auto signalName =
    [&configuration](SignalId signal)
  {
    const auto* definition =
      configuration.findSignalDefinition(signal);

    if (definition == nullptr)
      return std::to_string(signal.value);

    return definition->name;
  };

  for (auto p = it; p != path.end(); ++p)
  {
    if (p != it)
      message += " -> ";

    message += signalName(*p);
  }

  message += " -> ";
  message += signalName(id);

  fail(std::move(message));
}

std::span<const SignalId>
CalculationOrder::order() const noexcept
{
  return m_order;
}

}