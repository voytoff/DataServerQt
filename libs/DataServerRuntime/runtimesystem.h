#pragma once

#include "buffermanager.h"
#include "calculationplan.h"
#include "calibrationrepository.h"
#include "datablockqueue.h"
#include "dataengine.h"
#include "datastreamprocessor.h"
#include "datastreamreader.h"
#include "datastreamsourcemanager.h"
#include "datastreamworker.h"
#include "formulaastrepository.h"
#include "frameassembler.h"
#include "signalmemorylayout.h"
#include "signalprocessor.h"

#include <memory>

namespace qds
{

struct RuntimeSystem
{
  RuntimeSystem() = default;

  RuntimeSystem(
    const RuntimeSystem&) = delete;

  RuntimeSystem& operator=(
    const RuntimeSystem&) = delete;

  RuntimeSystem(
    RuntimeSystem&&) = delete;

  RuntimeSystem& operator=(
    RuntimeSystem&&) = delete;


  SignalMemoryLayout layout;

  FormulaAstRepository formulas;
  CalibrationRepository calibrations;
  CalculationPlan calculationPlan;

  BufferManager buffers;

  DataBlockQueue queue;
  DataStreamSourceManager dataSources;

  std::unique_ptr<SignalProcessor>
    signalProcessor;

  std::unique_ptr<DataStreamReader>
    streamReader;

  std::unique_ptr<FrameAssembler>
    frameAssembler;

  std::unique_ptr<DataStreamProcessor>
    streamProcessor;

  std::unique_ptr<DataStreamWorker>
    streamWorker;

  std::unique_ptr<DataEngine>
    engine;
};

}