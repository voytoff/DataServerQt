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
      return false;

    //dependencies.push_back(
    //  definition->id);
    if (std::find(
          dependencies.begin(),
          dependencies.end(),
          definition->id) == dependencies.end())
    {
      dependencies.push_back(
        definition->id);
    }

    const auto reference =
      m_layout.reference(
        definition->id);

    if (!reference.isValid())
      return false;

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
      return false;

    return resolveNode(
      *node.left,
      dependencies);
  }

  case FormulaNodeType::FunctionCall:
  {
    for (auto& argument : node.arguments)
    {
      if (!argument)
        return false;

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

  return false;
}

}