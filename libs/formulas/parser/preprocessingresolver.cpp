#include "preprocessingresolver.h"

namespace qds
{

bool PreprocessingResolver::resolve(
  FormulaNode& root)
{
  return resolveNode(root);
}

bool PreprocessingResolver::resolveNode(
  FormulaNode& node)
{
  switch (node.type)
  {
  case FormulaNodeType::Identifier:
    if (node.identifier != "x")
      return false;

    node.type = FormulaNodeType::Input;
    node.identifier.clear();
    return true;

  case FormulaNodeType::Number:
  case FormulaNodeType::Input:
    return true;

  case FormulaNodeType::Add:
  case FormulaNodeType::Subtract:
  case FormulaNodeType::Multiply:
  case FormulaNodeType::Divide:
    return node.left &&
           node.right &&
           resolveNode(*node.left) &&
           resolveNode(*node.right);

  case FormulaNodeType::Negate:
    return node.left &&
           resolveNode(*node.left);

  case FormulaNodeType::FunctionCall:
    for (auto& argument : node.arguments)
    {
      if (!argument ||
          !resolveNode(*argument))
        return false;
    }

    return true;

  case FormulaNodeType::Signal:
    return false;
  }

  return false;
}

}