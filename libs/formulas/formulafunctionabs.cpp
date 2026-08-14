#include "formulafunctionabs.h"

#include <cmath>
#include <span>

namespace qds
{

bool FormulaFunctionAbs::execute(
  std::span<const double> arguments,
  double &result) const noexcept
{
  if (arguments.size() != 1)
    return false;

  result = std::abs(arguments[0]);

  return true;
}

}