#include "systemclock.h"

#include <chrono>

namespace qds
{

Timestamp SystemClock::timestamp() const noexcept
{
  using namespace std::chrono;

  return {
    static_cast<uint64_t>(
      duration_cast<microseconds>(
        steady_clock::now().time_since_epoch())
        .count())
  };
}

// храним время в микросекундах
WallClockTime SystemClock::wallClockTime() const noexcept
{
  using namespace std::chrono;

  return {
    duration_cast<microseconds>(
      system_clock::now().time_since_epoch())
      .count()
  };
}

}