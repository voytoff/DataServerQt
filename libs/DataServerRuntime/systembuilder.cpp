#include "systembuilder.h"

#include "calculationcompiler.h"
#include "formulabuilder.h"

namespace qds
{

namespace
{

bool validateCalibrations(
  const CalculationPlan& plan,
  const SensorRepository& sensors,
  const CalibrationRepository& calibrations)
{
  for (const CalculationStep& step : plan.steps())
  {
    if (!step.sensorId)
      continue;

    const Sensor* sensor =
      sensors.find(step.sensorId);

    if (sensor == nullptr)
      return false;

    double result = 0.0;

    switch (sensor->calibrationMode)
    {
    case SensorCalibrationMode::Own:
      if (!calibrations.calibrateBySensor(
            sensor->id,
            sensor->calibrationKind,
            0.0,
            result))
        return false;
      break;

    case SensorCalibrationMode::ByType:
    {
      const SensorType* type =
        sensors.find(sensor->sensorTypeId);

      if (type == nullptr)
        return false;

      if (!calibrations.calibrateBySensorType(
            type->id,
            type->calibrationKind,
            0.0,
            result))
        return false;

      break;
    }

    default:
      return false;
    }
  }

  return true;
}

}

std::unique_ptr<RuntimeSystem>
SystemBuilder::build(
  SystemConfiguration& configuration,
  const DataStreamSourceFactory& dataSourceFactory,
  const SensorRepository &sensors,
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

  CalculationCompiler compiler(
    configuration,
    runtime->layout,
    runtime->formulas);

  if (!compiler.build(
      runtime->calculationPlan))
  {
    return nullptr;
  }

  if (!validateCalibrations(
      runtime->calculationPlan,
      sensors,
      calibrations))
  {
    return nullptr;
  }

  runtime->sensors = sensors;
  runtime->calibrations = calibrations;

  runtime->signalProcessor =
    std::make_unique<SignalProcessor>(
      runtime->layout,
      runtime->formulas,
      runtime->calculationPlan,
      runtime->sensors,
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