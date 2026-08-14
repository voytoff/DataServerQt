#pragma once

#include "iformulafunction.h"

  namespace qds
{

  class FormulaFunctionMax final
    : public IFormulaFunction
  {
  public:

    bool execute(
      std::span<const double> arguments,
      double& result) const noexcept override;
  };

}