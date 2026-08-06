#include "formulacopy.h"

namespace qds
{

bool FormulaCopy::execute(FormulaContext& ctx) const noexcept
{
  if (ctx.output == nullptr)
    return false;

  if (ctx.inputs.front() == nullptr)
    return false;

  if (ctx.inputs.size() < 1)
    return false;

  *ctx.output = *ctx.inputs[0];

  return true;
}

}