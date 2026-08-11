#pragma once

#include "iformula.h"

namespace qds
{

class FormulaSqrt : public IFormula
{
public:
  bool execute(FormulaContext& ctx) const noexcept override;

};

}
