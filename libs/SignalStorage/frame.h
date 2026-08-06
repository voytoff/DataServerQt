#pragma once

#include <cassert>

#include "signalmemory.h"
#include "signalmemorylayout.h"
#include "datatypes.h"

namespace qds
{

class Frame
{
public:

  void initialize(
    const SignalMemoryLayout& layout);


  [[nodiscard]]
  double* address(
    SignalReference ref) noexcept;


  [[nodiscard]]
  const double* address(
    SignalReference ref) const noexcept;


  RawMemory& raw() noexcept;

  CalculatedMemory& calculated() noexcept;


  FrameNumber number;

  Timestamp timestamp;

  WallClockTime wallTime;


private:

  RawMemory m_raw;

  CalculatedMemory m_calculated;
};

}