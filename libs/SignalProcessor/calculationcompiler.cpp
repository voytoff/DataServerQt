#include "calculationcompiler.h"
#include <cassert>
#include <queue>

namespace qds
{

CalculationCompiler::CalculationCompiler(
  const SystemConfiguration &cfg,
  const SignalMemoryLayout &layout, const FormulaRepository &repository)
  : m_cfg(cfg)
  , m_layout(layout)
  , m_repository(repository) { }

bool CalculationCompiler::build(
  CalculationPlan& plan)
{
  plan.clear();
  m_nodes.clear();
  m_index.clear();

  if (!buildNodes())
    return false;

  connectNodes();

  if (!topologicalSort(plan))
  {
    plan.clear();
    return false;
  }

  return true;
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
    node.formula = definition.formulaId;

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

void CalculationCompiler::connectNodes()
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
        return; // неверная конфигурация

      auto& parent = m_nodes[it->second];

      parent.children.push_back(&node);
    }
  }
}


bool CalculationCompiler::topologicalSort(CalculationPlan& plan)
{
  std::deque<Node*> queue;

  for (auto& node : m_nodes)
  {
    if (node.indegree == 0)
      queue.push_back(&node);
  }

  size_t processedCount = 0;
  CalculationPlan tmp;
  tmp.reserve(m_nodes.size());

  while (!queue.empty())
  {
    Node* node = queue.front();
    queue.pop_front();

    CalculationStep step;

    step.formula = m_repository.find(node->formula);

    step.output = m_layout.reference(node->id);

    if (step.formula == nullptr)
      return false;

    step.inputs.reserve(node->dependencies.size());
    for (auto& id : node->dependencies)
    {
      step.inputs.push_back(
        m_layout.reference(id));
    }

    for (Node* child : node->children)
    {
      assert(child->indegree > 0);

      --child->indegree;

      if (child->indegree == 0)
        queue.push_back(child);
    }

    tmp.addStep(std::move(step));

    ++processedCount;
  }

  if (processedCount != m_nodes.size())
    return false;

  plan = tmp;
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
