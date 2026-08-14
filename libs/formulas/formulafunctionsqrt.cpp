#include "formulafunctionsqrt.h"

#include <cmath>
#include <span>

namespace qds
{

bool FormulaFunctionSqrt::execute(
  std::span<const double> arguments,
  double& result) const noexcept
{
  if (arguments.size() != 1)
    return false;

  if (arguments[0] < 0.0)
    return false;

  result = std::sqrt(arguments[0]);

  return true;
}

}