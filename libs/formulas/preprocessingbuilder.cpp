#include "preprocessingbuilder.h"

#include "parser/formulaparser.h"
#include "parser/preprocessingresolver.h"
#include "parser/preprocessingresolver.h"

#include <utility>
#include <vector>

namespace qds
{

bool PreprocessingBuilder::build(
  SystemConfiguration& configuration,
  FormulaAstRepository& formulas)
{
  FormulaAstRepository tmp;

  struct RuntimeData
  {
    SignalId signal;
    FormulaId formula;
  };

  std::vector<RuntimeData> runtimeData;

  PreprocessingResolver resolver;

  for (const auto& definition :
       configuration.signalDefinitions())
  {
    if (definition.kind != SignalKind::Raw)
      continue;

    if (definition.preprocessingFormula.empty())
      continue;

    FormulaParser parser(
      definition.preprocessingFormula);

    auto node = parser.parse();

    if (!node)
      return false;

    if (!resolver.resolve(*node))
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
      .formula = formulaId
    });
  }

  // Обновляем конфигурацию только после
  // успешной компиляции всех формул.
  for (const auto& data : runtimeData)
  {
    auto* definition =
      configuration.findSignalDefinition(
        data.signal);

    if (!definition)
      return false;

    definition->preprocessingFormulaId =
      data.formula;
  }

  formulas = std::move(tmp);

  return true;
}

}