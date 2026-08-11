#pragma once

#include "iformula.h"

namespace qds
{

class FormulaTest : public IFormula
{
public:
  bool execute(FormulaContext& ctx) const noexcept override;

  FormulaContext &context() const;

private:
  FormulaContext m_context;
};

}


