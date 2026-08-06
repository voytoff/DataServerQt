#pragma once

namespace qds
{

class Frame;

class ICalculationProcessor
{
public:

  virtual ~ICalculationProcessor() = default;

  [[nodiscard]]
  virtual bool process(
    Frame& frame) = 0;
};

}