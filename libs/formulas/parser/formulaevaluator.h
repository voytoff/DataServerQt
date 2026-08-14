#pragma once

#include "formulanode.h"
#include "formulafunctionrepository.h"
#include "signalmemory.h"

namespace qds
{

class FormulaEvaluator
{
public:

  explicit FormulaEvaluator(
    const FormulaFunctionRepository& functions);

  [[nodiscard]]
  bool evaluate(
    const FormulaNode& node,
    const RawMemory& raw,
    const CalculatedMemory& calculated,
    double& result) const noexcept;

private:

  bool evaluateNode(
    const FormulaNode& node,
    const RawMemory& raw,
    const CalculatedMemory& calculated,
    double& result) const noexcept;

  bool readSignal(
    const SignalReference& signal,
    const RawMemory& raw,
    const CalculatedMemory& calculated,
    double& value) const noexcept;

private:

  const FormulaFunctionRepository& m_functions;
};

}