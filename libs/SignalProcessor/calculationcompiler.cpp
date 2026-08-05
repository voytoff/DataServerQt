#include "calculationcompiler.h"
#include <cassert>

namespace qds
{

CalculationCompiler::CalculationCompiler(
  const SystemConfiguration &cfg,
  const SignalMemoryLayout &layout)
  : m_cfg(cfg)
  , m_layout(layout) { }

bool CalculationCompiler::build(
  CalculationPlan& plan)
{
  plan.clear();
  m_nodes.clear();
  m_index.clear();

  if (!buildNodes())
    return false;

  if (!connectNodes())
    return false;

  return topologicalSort(plan);
}

bool CalculationCompiler::buildNodes()
{
  for (const auto& definition : m_cfg.signalDefinitions())
  {
    if (definition.kind != SignalKind::Calculated)
      continue;

    m_nodes.emplace_back();

    Node& node =
      m_nodes.back();

    node.id = definition.id;

    node.dependencies = definition.dependencies;

    for (auto &p : node.dependencies)
    {
      if (isCalculatedSignal(p))
        ++node.indegree;
    }

    auto [it, inserted] = m_index.emplace(
      node.id,
      m_nodes.size() - 1);

    assert(inserted);
  }

  return true;
}

bool CalculationCompiler::connectNodes()
{
  for (auto &node : m_nodes)
  {
    for (auto &p : node.dependencies)
    {
      if (!isCalculatedSignal(p))
        continue;

      auto it = m_index.find(p);

      assert(it != m_index.end());
      if (it == m_index.end())
        return false; // неверная конфигурация

      auto& parent = m_nodes[it->second];

      parent.dependents.push_back(node.id);    }
  }

  return true;
}

bool CalculationCompiler::topologicalSort(CalculationPlan& plan)
{
  return true;
}

bool CalculationCompiler::isCalculatedSignal(SignalId id) const
{
  const auto ref =
    m_layout.reference(id);

  return ref.area ==
         SignalMemoryArea::Calculated;
}

}
