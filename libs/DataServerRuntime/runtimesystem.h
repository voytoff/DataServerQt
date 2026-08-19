#pragma once

#include <memory>

#include "buffermanager.h"
#include "calculationplan.h"
#include "datasourcemanager.h"
#include "formulaastrepository.h"
#include "signalmemorylayout.h"
#include "signalprocessor.h"

#include "dataengine.h"

namespace qds
{

struct RuntimeSystem
{
  SignalMemoryLayout layout;

  FormulaAstRepository formulas;
  CalculationPlan calculationPlan;

  DataSourceManager dataSources;
  BufferManager buffers;

  std::unique_ptr<SignalProcessor> signalProcessor;
  std::unique_ptr<DataEngine> engine;
};

}