#include "formulaadd.h"

namespace qds
{

bool FormulaAdd::execute(FormulaContext& ctx) const noexcept
{
  if (ctx.output == nullptr)
    return false;

  if (ctx.inputs.front() == nullptr)
    return false;

  if (ctx.inputs.size() < 1)
    return false;

  double sum = 0.0;

  for (size_t i = 0; i < ctx.inputs.size(); ++i)
    sum += *ctx.inputs[i];

  ctx.output = &sum;

  return true;
}

}