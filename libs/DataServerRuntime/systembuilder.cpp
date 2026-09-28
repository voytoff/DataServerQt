#include "systembuilder.h"

#include "calculationcompiler.h"
#include "formulabuilder.h"

namespace qds
{

std::unique_ptr<RuntimeSystem>
SystemBuilder::build(
  SystemConfiguration& configuration,
  const DataStreamSourceFactory& dataSourceFactory,
  const CalibrationRepository& calibrations,
  IClock& clock,
  IArchiveWriter& archive,
  ILogger& logger)
{
  auto runtime =
    std::make_unique<RuntimeSystem>();

  runtime->layout.build(
    configuration);

  FormulaBuilder formulaBuilder;

  if (!formulaBuilder.build(
        configuration,
        runtime->layout,
        runtime->formulas))
  {
    return nullptr;
  }

  runtime->calibrations =
    calibrations;

  CalculationCompiler compiler(
    configuration,
    runtime->layout,
    runtime->formulas);

  if (!compiler.build(
        runtime->calculationPlan))
  {
    return nullptr;
  }

  runtime->signalProcessor =
    std::make_unique<SignalProcessor>(
      runtime->layout,
      runtime->formulas,
      runtime->calculationPlan,
      runtime->calibrations);

  runtime->buffers.initialize(
    runtime->layout);

  runtime->streamReader =
    std::make_unique<DataStreamReader>();

  runtime->frameAssembler =
    std::make_unique<FrameAssembler>(
      configuration,
      runtime->layout,
      FrameStartPolicy::WaitForAllModules);

  runtime->streamProcessor =
    std::make_unique<DataStreamProcessor>(
      *runtime->streamReader,
      *runtime->frameAssembler,
      *runtime->signalProcessor,
      runtime->buffers,
      archive,
      logger);

  runtime->streamWorker =
    std::make_unique<DataStreamWorker>(
      runtime->queue,
      *runtime->streamProcessor,
      logger);

  if (!runtime->dataSources.initialize(
        configuration,
        dataSourceFactory,
        clock,
        runtime->queue,
        runtime->queue))
  {
    return nullptr;
  }

  runtime->engine =
    std::make_unique<DataEngine>();

  return runtime;
}

}