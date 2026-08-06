#pragma once

#include "icalculationprocessor.h"

namespace qds
{

class EmptySignalProcessor : public ICalculationProcessor
{
public:
  bool process(Frame& frame) override;
  void applyCalibrations(Frame& frame);
  void evaluateFormulas(Frame& frame);
};

}
