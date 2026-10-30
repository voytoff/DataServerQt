#include "identifierresolver.h"

namespace qds
{

IdentifierResolver::IdentifierResolver(
  const SystemConfiguration &cfg,
  const SignalMemoryLayout &layout)
  : m_cfg(cfg)
  , m_layout(layout) { }

bool IdentifierResolver::resolve(
  FormulaNode& root,
  std::vector<SignalId>& dependencies)
{
  m_lastError.clear();
  dependencies.clear();

  return resolveNode(
    root,
    dependencies);
}

bool IdentifierResolver::resolveNode(
  FormulaNode& node,
  std::vector<SignalId>& dependencies)
{
  switch (node.type)
  {
  case FormulaNodeType::Identifier:
  {
    const auto* definition =
      m_cfg.findSignalDefinition(
        node.identifier);

    if (definition == nullptr)
    {
      return fail(
        "Неизвестный сигнал: " +
        node.identifier);
    }

    const auto reference =
      m_layout.reference(
        definition->id);

    if (!reference.isValid())
    {
      return fail(
        "Сигнал отсутствует в памяти: " +
        node.identifier +
        " (SignalId: " +
        std::to_string(definition->id.value) +
        ")");
    }

    if (std::find(
          dependencies.begin(),
          dependencies.end(),
          definition->id) == dependencies.end())
    {
      dependencies.push_back(
        definition->id);
    }

    node.signal = reference;
    node.type = FormulaNodeType::Signal;

    return true;
  }

  case FormulaNodeType::Number:
    return true;

  case FormulaNodeType::Add:
  case FormulaNodeType::Subtract:
  case FormulaNodeType::Multiply:
  case FormulaNodeType::Divide:
  {
    if (!node.left || !node.right)
      return false;

    return
      resolveNode(*node.left, dependencies) &&
      resolveNode(*node.right, dependencies);
  }

  case FormulaNodeType::Negate:
  {
    if (!node.left)
      return fail("Отсутствует операнд унарной операции");

    return resolveNode(
      *node.left,
      dependencies);
  }

  case FormulaNodeType::FunctionCall:
  {
    for (auto& argument : node.arguments)
    {
      if (!argument)
        return fail("Отсутствует аргумент функции");

      if (!resolveNode(
            *argument,
            dependencies))
        return false;
    }

    return true;
  }

  case FormulaNodeType::Signal:
    return true;
  }

return fail("Неизвестный тип узла формулы");
}

bool IdentifierResolver::fail(std::string message)
{
  if (m_lastError.empty())
    m_lastError = std::move(message);

  return false;
}

const std::string&
IdentifierResolver::lastError() const noexcept
{
  return m_lastError;
}

}