#pragma once

#include "iformula.h"

namespace qds
{

class FormulaAdd : public IFormula
{
public:
  bool execute(FormulaContext& ctx) const noexcept override;

};

}
