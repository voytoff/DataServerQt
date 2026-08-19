#include "formulabuilder.h"

#include "parser/formulaparser.h"
#include "parser/identifierresolver.h"

namespace qds
{

bool FormulaBuilder::build(
  SystemConfiguration& configuration,
  const SignalMemoryLayout& layout,
  FormulaAstRepository& formulas)
{
  FormulaAstRepository tmp;

  struct RuntimeData
  {
    SignalId signal;
    FormulaId formula;
    std::vector<SignalId> dependencies;
  };

  std::vector<RuntimeData> runtimeData;

  IdentifierResolver resolver(
    configuration,
    layout);

  for (auto& definition :
       configuration.signalDefinitions())
  {
    if (definition.kind != SignalKind::Calculated)
      continue;

    FormulaParser parser(
      definition.formula);

    auto node = parser.parse();

    if (node == nullptr)
      return false;

    std::vector<SignalId> dependencies;

    if (!resolver.resolve(
          *node,
          dependencies))
      return false;

    const FormulaId formulaId{
      definition.id.value
    };

    if (!tmp.add(
          formulaId,
          std::move(node)))
      return false;

    runtimeData.push_back({
      .signal = definition.id,
      .formula = formulaId,
      .dependencies = std::move(dependencies)
    });
  }

  // Только после полного успеха
  // изменяем configuration.
  for (const auto& data : runtimeData)
  {
    auto* definition =
      configuration.findSignalDefinition(
        data.signal);

    if (definition == nullptr)
      return false;

    definition->formulaId =
      data.formula;

    definition->dependencies =
      data.dependencies;
  }

  formulas = std::move(tmp);

  return true;
}

}