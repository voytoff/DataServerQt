#pragma once

#include <strongidhash.h>
#include <unordered_map>
#include "calculationplan.h"
#include "formularepository.h"
#include "signalmemorylayout.h"
#include "systemconfiguration.h"

namespace qds
{

class CalculationCompiler
{
public:

  explicit CalculationCompiler(
    const SystemConfiguration& cfg,
    const SignalMemoryLayout& layout,
    const FormulaRepository& repository);

  bool build(
    CalculationPlan& plan);

private:

  struct Node
  {
    SignalId id;

    FormulaId formula;

    std::vector<SignalId> dependencies;
    std::vector<Node*> children;

    uint32_t indegree = 0;
  };

private:

  bool buildNodes();

  bool connectNodes();

  bool topologicalSort(
    CalculationPlan& plan);

  bool isCalculatedSignal(
    SignalId id) const;

private:

  const SystemConfiguration& m_cfg;
  const SignalMemoryLayout& m_layout;
  const FormulaRepository& m_repository;

  std::vector<Node> m_nodes;
  std::unordered_map<SignalId, size_t> m_index;
};

}