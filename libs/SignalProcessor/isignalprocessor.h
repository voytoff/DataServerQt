#pragma once

#include "frame.h"

namespace qds
{

class ISignalProcessor
{
public:
  virtual ~ISignalProcessor() = default;

  virtual void process(Frame& frame) = 0;
};

}