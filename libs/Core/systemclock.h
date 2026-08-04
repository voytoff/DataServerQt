#pragma once

#include "iclock.h"

namespace qds
{

class SystemClock final : public IClock
{
public:
  Timestamp timestamp() const noexcept override;

  WallClockTime wallClockTime() const noexcept override;
};

}