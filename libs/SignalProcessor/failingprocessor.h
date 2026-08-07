#pragma once

#include "icalculationprocessor.h"

namespace qds
{

class FailingProcessor : public ICalculationProcessor
{
public:

  bool process(Frame&) override
  {
    return false;
  }

};

}
