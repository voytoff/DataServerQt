#pragma once

#include <strongidhash.h>
#include <unordered_map>
#include "calculationplan.h"
#include "signalmemorylayout.h"
#include "systemconfiguration.h"

namespace qds
{

class CalculationCompiler
{
public:

  explicit CalculationCompiler(
    const SystemConfiguration& cfg,
    const SignalMemoryLayout& layout);

  bool build(
    CalculationPlan& plan);

private:

  struct Node
  {
    SignalId id;

    FormulaId formula;

    std::vector<SignalId> dependencies;
    std::vector<Node*> dependents;
    //std::vector<SignalId> dependents;

    uint32_t indegree = 0;

#ifndef NDEBUG
    bool emitted = false;
#endif
  };

private:

  bool buildNodes();

  void connectNodes();

  bool topologicalSort(
    CalculationPlan& plan);

  bool isCalculatedSignal(
    SignalId id) const;

private:

  const SystemConfiguration& m_cfg;
  const SignalMemoryLayout& m_layout;

  std::vector<Node> m_nodes;
  std::unordered_map<SignalId, size_t> m_index;
};

}