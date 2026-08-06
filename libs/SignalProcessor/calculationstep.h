#pragma once

#include "iformula.h"
#include "signaldefinition.h"

namespace qds
{

// DSL (Domain Specific Language) для обработки сигналов
// команда виртуальной машины.
struct CalculationStep
{
  SignalReference output;

  std::vector<SignalReference> inputs;

  const IFormula* formula = nullptr;
};

}