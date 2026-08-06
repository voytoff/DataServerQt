#pragma once

#include "formulacontext.h"

namespace qds
{

struct CalculationStep;
class Frame;

class IFormula
{
public:

  virtual ~IFormula() noexcept = default;

  [[nodiscard]]
  virtual bool execute(FormulaContext& ctx) = 0;
};

}