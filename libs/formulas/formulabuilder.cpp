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
  m_lastError.clear();

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

    const std::string prefix =
      "Ошибка формулы сигнала \"" +
      definition.name +
      "\" (SignalId: " +
      std::to_string(definition.id.value) +
      "): ";

    FormulaParser parser(
      definition.formula);

    auto node = parser.parse();

    if (!node)
    {
      m_lastError =
        prefix + parser.lastError();

      return false;
    }

    std::vector<SignalId> dependencies;

    if (!resolver.resolve(
          *node,
          dependencies))
    {
      m_lastError =
        prefix + resolver.lastError();

      return false;
    }

    const FormulaId formulaId{
      definition.id.value
    };

    if (!tmp.add(
          formulaId,
          std::move(node)))
    {
      m_lastError =
        prefix + "Не удалось зарегистрировать формулу";

      return false;
    }

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

    if (!definition)
    {
      m_lastError =
        "Не найден вычисляемый сигнал (SignalId: " +
        std::to_string(data.signal.value) +
        ")";

      return false;
    }

    definition->formulaId =
      data.formula;

    definition->dependencies =
      data.dependencies;
  }

  formulas = std::move(tmp);

  return true;
}

const std::string&
FormulaBuilder::lastError() const noexcept
{
  return m_lastError;
}

}