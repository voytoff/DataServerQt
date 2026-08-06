#pragma once

#include "datatypes.h"

namespace qds
{

struct FormulaContext
{
  Sample* output;
  const Sample* const* inputs;
  size_t inputCount;
};

}