#include "systembuilder.h"

#include "calculationcompiler.h"
#include "formulabuilder.h"

namespace qds
{

bool SystemBuilder::build(
  SystemConfiguration& configuration,
  const DataSourceFactory& dataSourceFactory,
  RuntimeSystem& runtime)
{
  runtime = {};

  runtime.layout.build(configuration);

  FormulaBuilder formulaBuilder;

  if (!formulaBuilder.build(
        configuration,
        runtime.layout,
        runtime.formulas))
  {
    runtime = {};
    return false;
  }

  CalculationCompiler compiler(
    configuration,
    runtime.layout,
    runtime.formulas);

  if (!compiler.build(
        runtime.calculationPlan))
  {
    runtime = {};
    return false;
  }

  runtime.signalProcessor =
    std::make_unique<SignalProcessor>(
      runtime.layout,
      runtime.formulas,
      runtime.calculationPlan);

  if (!runtime.dataSources.initialize(
        configuration,
        runtime.layout,
        dataSourceFactory))
  {
    runtime = {};
    return false;
  }

  runtime.buffers.initialize(
    runtime.layout);

  runtime.engine =
    std::make_unique<DataEngine>();

  return true;
}

}