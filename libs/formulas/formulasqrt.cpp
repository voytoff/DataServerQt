#include "formulasqrt.h"
#include <cmath>

namespace qds
{

bool FormulaSqrt::execute(FormulaContext& ctx) const noexcept
{
  if (ctx.output == nullptr)
    return false;

  if (ctx.inputs.size() != 1)
    return false;

  if (ctx.inputs[0] == nullptr)
    return false;

  *ctx.output = std::sqrt(*ctx.inputs[0]);

  return true;
}

}