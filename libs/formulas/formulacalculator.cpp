#include "formulacalculator.h"
#include "formulafunctionabs.h"
#include "formulafunctionmax.h"
#include "formulafunctionmin.h"
#include "formulafunctionsqrt.h"
#include "parser/identifierresolver.h"

namespace qds
{

FormulaCalculator::FormulaCalculator()
  : m_evaluator(m_functions) { }

bool FormulaCalculator::initialize(
  const SystemConfiguration &configuration,
  const SignalMemoryLayout &layout,
  FormulaAstRepository &formulas)
{
  m_configuration = &configuration;
  m_layout = &layout;
  m_formulas = &formulas;

  IdentifierResolver resolver(
    configuration,
    layout);

  for (const auto& definition :
       configuration.signalDefinitions())
  {
    if (definition.kind != SignalKind::Calculated)
      continue;

    auto* node =
      formulas.find(definition.formulaId);

    if (node == nullptr)
      return false;

    if (!resolver.resolve(*node))
      return false;
  }

  if (!m_order.build(configuration))
    return false;

  if (!m_functions.add("abs", std::make_unique<FormulaFunctionAbs>()) ||
      !m_functions.add("max", std::make_unique<FormulaFunctionMax>()) ||
      !m_functions.add("min", std::make_unique<FormulaFunctionMin>()) ||
      !m_functions.add("sqrt", std::make_unique<FormulaFunctionSqrt>()))
    return false;

  return true;
}

bool FormulaCalculator::calculate(
  const RawMemory& raw,
  CalculatedMemory& calculated) const noexcept
{
  if (m_configuration == nullptr ||
      m_layout == nullptr ||
      m_formulas == nullptr)
    return false;

  double result = 0.0;

  for (SignalId id : m_order.order())
  {
    const auto* definition =
      m_configuration->findSignalDefinition(id);

    if (definition == nullptr)
      return false;

    const auto* ast =
      m_formulas->find(definition->formulaId);

    if (ast == nullptr)
      return false;

    if (!m_evaluator.evaluate(
          *ast,
          raw,
          calculated,
          result))
      return false;

    const auto reference =
      m_layout->reference(id);

    if (!reference.isValid())
      return false;

    calculated.setValue(
      reference.index,
      result);
  }

  return true;
}

}