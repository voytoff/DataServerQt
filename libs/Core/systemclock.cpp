#include "systemclock.h"
#include <chrono>

namespace qds
{

uint64_t SystemClock::now() const noexcept
{
  using namespace std::chrono;

  return duration_cast<milliseconds>(
    system_clock::now().time_since_epoch())
      .count();
}

}