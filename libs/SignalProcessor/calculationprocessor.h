#pragma once

#include "icalculationprocessor.h"
#include "calculationplan.h"
#include "formulacontextbuilder.h"

namespace qds
{

class CalculationProcessor final : public ICalculationProcessor
{
public:

  explicit CalculationProcessor(
    const CalculationPlan& plan);


  [[nodiscard]]
  bool process(
    Frame& frame) override;


private:

  const CalculationPlan& m_plan;

  FormulaContextBuilder m_contextBuilder;
};

}