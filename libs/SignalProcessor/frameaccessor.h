#pragma once

#include "frame.h"
#include "signaldefinition.h"

namespace qds
{

class FrameAccessor
{
public:

  [[nodiscard]]
  static double read(
    const Frame& frame,
    SignalReference reference) noexcept;


  static void write(
    Frame& frame,
    SignalReference reference,
    double value) noexcept;
};

}