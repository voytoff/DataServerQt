#pragma once

#include "iformula.h"

namespace qds
{

class FormulaCopy : public IFormula
{
public:
  bool execute(FormulaContext& ctx) const noexcept override;

};

}


