#include "formulafunctionmin.h"

#include <algorithm>
#include <span>

namespace qds
{

bool FormulaFunctionMin::execute(
  std::span<const double> arguments,
  double &result) const noexcept
{
  if (arguments.size() < 2)
    return false;

  result = *std::min_element(arguments.begin(), arguments.end());

  return true;
}

}