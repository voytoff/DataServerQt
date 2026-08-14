#include "formulafunctionmax.h"

#include <algorithm>
#include <span>

namespace qds
{

bool FormulaFunctionMax::execute(
  std::span<const double> arguments,
  double &result) const noexcept
{
  if (arguments.size() < 2)
    return false;

  result = *std::max_element(arguments.begin(), arguments.end());

  return true;
}

}