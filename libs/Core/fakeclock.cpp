#include "fakeclock.h"

namespace qds
{

void FakeClock::set(uint64_t t)
{
  m_now = t;
}

void FakeClock::advance(uint64_t dt)
{
  m_now += dt;
}

uint64_t FakeClock::now() const noexcept
{
  return m_now;
}

}