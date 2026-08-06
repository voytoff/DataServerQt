#include "formulacopy.h"

namespace qds
{

bool FormulaCopy::execute(FormulaContext& ctx)
{
  if (ctx.output == nullptr)
    return false;

  if (ctx.inputs == nullptr)
    return false;

  if (ctx.inputCount < 1)
    return false;

  *ctx.output = *ctx.inputs[0];

  return true;
}

}