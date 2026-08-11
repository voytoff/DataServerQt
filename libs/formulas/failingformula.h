#pragma once

#include "iformula.h"
namespace qds
{

class FailingFormula : public IFormula
{
public:
  bool execute(FormulaContext&) const noexcept override
  {
    return false;
  }
};

}