#pragma once

#include "iclock.h"

namespace qds
{

class FakeClock final : public IClock
{
public:

  void setTimestamp(uint64_t value);
  void setWallClockTime(int64_t value);

  void advance(uint64_t microseconds);

  [[nodiscard]]
  Timestamp timestamp() const noexcept override;

  [[nodiscard]]
  WallClockTime wallClockTime() const noexcept override;


private:

  uint64_t m_timestamp = 0;
  int64_t m_wallTime = 0;

};

}