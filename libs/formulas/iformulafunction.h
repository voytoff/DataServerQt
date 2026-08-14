#pragma once

#include <span>

namespace qds
{

class IFormulaFunction
{
public:
  virtual ~IFormulaFunction() noexcept = default;

  [[nodiscard]]
  virtual bool execute(
    std::span<const double> arguments,
    double& result) const noexcept = 0;
};

}