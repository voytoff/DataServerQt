#pragma once

#include "frame.h"

namespace qds
{

class ISignalProcessor
{
public:
  virtual ~ISignalProcessor() = default;

  [[nodiscard]]
  virtual bool process(Frame& frame) = 0;
};

}