#include "systembuilder.h"
#include "configurationvalidator.h"

namespace qds
{

std::unique_ptr<RuntimeSystem>
SystemBuilder::build(
  SystemConfiguration& configuration,
  const DataStreamSourceFactory& dataSourceFactory,
  const SensorRepository &sensors,
  const CalibrationRepository& calibrations,
  const ModuleConfigurationValidatorRegistry& moduleValidators,
  IClock& clock,
  IArchiveWriter& archive,
  ILogger& logger)
{
  auto runtime =
    std::make_unique<RuntimeSystem>();

  ConfigurationValidator validator;
  PreparedConfiguration prepared;

  if (!validator.build(
        configuration,
        sensors,
        calibrations,
        moduleValidators,
        prepared))
  {
    logger.error(
      "Ошибка конфигурации: " +
      validator.lastError());

    return nullptr;
  }

  runtime->layout =
    std::move(prepared.layout);

  runtime->formulas =
    std::move(prepared.formulas);

  runtime->calculationPlan =
    std::move(prepared.calculationPlan);

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
        runtime->queue,
        logger))
  {
    logger.error(
      "Ошибка инициализации источников данных");

    return nullptr;
  }

  runtime->engine =
    std::make_unique<DataEngine>();

  return runtime;
}

}