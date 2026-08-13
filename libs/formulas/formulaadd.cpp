#include "formulaadd.h"

namespace qds
{

bool FormulaAdd::execute(FormulaContext& ctx) const noexcept
{
  if (ctx.output == nullptr)
    return false;

  if (ctx.inputs.empty())
    return false;

  double sum = 0.0;

  for (double* input : ctx.inputs)
  {
    if (input == nullptr)
      return false;

    sum += *input;
  }

  *ctx.output = sum;

  return true;
}

}