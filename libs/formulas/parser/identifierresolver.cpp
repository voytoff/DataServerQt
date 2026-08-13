#include "identifierresolver.h"

namespace qds
{

IdentifierResolver::IdentifierResolver(
  const SystemConfiguration &cfg,
  const SignalMemoryLayout &layout)
  : m_cfg(cfg)
  , m_layout(layout) { }

bool IdentifierResolver::resolve(
  FormulaNode& root)
{
  return resolveNode(root);
}

bool IdentifierResolver::resolveNode(
  FormulaNode& node)
{
  switch (node.type)
  {
  case FormulaNodeType::Identifier:
  {
    const auto* definition =
      m_cfg.findSignalDefinition(node.identifier);

    if (definition == nullptr)
      return false;

    const auto reference =
      m_layout.reference(definition->id);

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

    return resolveNode(*node.left) &&
           resolveNode(*node.right);
  }

  case FormulaNodeType::Negate:
  {
    if (!node.left)
      return false;

    return resolveNode(*node.left);
  }

  case FormulaNodeType::FunctionCall:
  {
    for (auto& argument : node.arguments)
    {
      if (!argument)
        return false;

      if (!resolveNode(*argument))
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