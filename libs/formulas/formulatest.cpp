#include "formulatest.h"

namespace qds
{

bool FormulaTest::execute(FormulaContext& ctx) const noexcept
{
  m_context.inputs = ctx.inputs;
  m_context.output = ctx.output;

  *ctx.output = 123.45;

  return true;
}

FormulaContext &FormulaTest::context() const
{
  return m_context;
}

}