#pragma once

namespace qds
{

struct CalculationStep;
class Frame;

class IFormula
{
public:

  virtual ~IFormula() noexcept = default;

  [[nodiscard]]
  virtual bool execute(
    Frame& frame,
    const CalculationStep& step) = 0;
};

}