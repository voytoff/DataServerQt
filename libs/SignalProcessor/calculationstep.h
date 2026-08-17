#pragma once

#include "signaldefinition.h"

namespace qds
{

struct CalculationStep
{
  SignalId signal;
  FormulaId formula;
};

}