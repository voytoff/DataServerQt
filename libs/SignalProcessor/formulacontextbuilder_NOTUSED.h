#pragma once

#include <vector>

#include "formulacontext.h"
#include "frame.h"
#include "calculationstep.h"

namespace qds
{

class FormulaContextBuilder
{
public:

  [[nodiscard]]
  FormulaContext create(
    Frame& frame,
    const CalculationStep& step);

private:

  std::vector<double*> m_inputs;
};

}