#pragma once

#include "isignalprocessor.h"

namespace qds
{

class EmptySignalProcessor : public ISignalProcessor
{
public:
  bool process(Frame& frame) override;
  void applyCalibrations(Frame& frame);
  void evaluateFormulas(Frame& frame);
};

}
