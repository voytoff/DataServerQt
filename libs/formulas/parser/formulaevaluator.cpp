#include "formulaevaluator.h"

namespace qds
{

FormulaEvaluator::FormulaEvaluator(
  const FormulaFunctionRepository &functions)
  : m_functions(functions) { }

bool FormulaEvaluator::evaluate(
  const FormulaNode& node,
  const RawMemory& raw,
  const CalculatedMemory& calculated,
  double& result) const noexcept
{
  return evaluateNode(
    node, &raw, &calculated, nullptr, result);
}

bool FormulaEvaluator::evaluate(
  const FormulaNode& node,
  double x,
  double& result) const noexcept
{
  return evaluateNode(
    node, nullptr, nullptr, &x, result);
}

bool FormulaEvaluator::evaluateNode(
  const FormulaNode& node,
  const RawMemory* raw,
  const CalculatedMemory* calculated,
  const double* input,
  double& result) const noexcept
{
  switch (node.type)
  {
  case FormulaNodeType::Number:
    result = node.number;
    return true;

  case FormulaNodeType::Signal:
    if (raw == nullptr ||
        calculated == nullptr ||
        !node.signal.isValid())
      return false;

    return readSignal(
      node.signal,
      *raw,
      *calculated,
      result);

  case FormulaNodeType::Add:
  {
    if (!node.left || !node.right)
      return false;

    double left;
    double right;

    if (!evaluateNode(
          *node.left,
          raw,
          calculated,
          input,
          left))

      return false;

    if (!evaluateNode(
          *node.right,
          raw,
          calculated,
          input,
          right))

      return false;

    result = left + right;
    return true;
  }

  case FormulaNodeType::Subtract:
  {
    if (!node.left || !node.right)
      return false;

    double left;
    double right;

    if (!evaluateNode(
          *node.left,
          raw,
          calculated,
          input,
          left))

      return false;

    if (!evaluateNode(
          *node.right,
          raw,
          calculated,
          input,
          right))

      return false;

    result = left - right;
    return true;
  }

  case FormulaNodeType::Multiply:
  {
    if (!node.left || !node.right)
      return false;

    double left;
    double right;

    if (!evaluateNode(
          *node.left,
          raw,
          calculated,
          input,
          left))

      return false;

    if (!evaluateNode(
          *node.right,
          raw,
          calculated,
          input,
          right))

      return false;

    result = left * right;
    return true;
  }

  case FormulaNodeType::Divide:
  {
    if (!node.left || !node.right)
      return false;

    double left;
    double right;

    if (!evaluateNode(
          *node.left,
          raw,
          calculated,
          input,
          left))

      return false;

    if (!evaluateNode(
          *node.right,
          raw,
          calculated,
          input,
          right))

      return false;

    if (right == 0.0)
      return false;

    result = left / right;
    return true;
  }

  case FormulaNodeType::Negate:
  {
    if (!node.left)
      return false;

    double value;

    if (!evaluateNode(
          *node.left,
          raw,
          calculated,
          input,
          value))

      return false;

    result = -value;
    return true;
  }

  case FormulaNodeType::FunctionCall:
  {
    const auto* function =
      m_functions.find(node.identifier);

    if (function == nullptr)
      return false;

    std::vector<double> arguments;
    arguments.reserve(node.arguments.size());

    for (const auto& argument : node.arguments)
    {
      if (!argument)
        return false;

      double value = 0.0;

      if (!evaluateNode(
            *argument,
            raw,
            calculated,
            input,
            value))
      {
        return false;
      }

      arguments.push_back(value);
    }

    return function->execute(
      arguments,
      result);
  }
  case FormulaNodeType::Identifier:
    return false;

  }

  return false;
}

bool FormulaEvaluator::readSignal(
  const SignalReference& signal,
  const RawMemory& raw,
  const CalculatedMemory& calculated,
  double& result) const noexcept
{
  switch (signal.area)
  {
  case SignalMemoryArea::Raw:
    if (signal.index >= raw.size())
      return false;

    result = raw.value(signal.index);
    return true;

  case SignalMemoryArea::Calculated:
    if (signal.index >= calculated.size())
      return false;

    result = calculated.value(signal.index);
    return true;

  default:
    return false;
  }
}

}
