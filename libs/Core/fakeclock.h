#pragma once

#include "iclock.h"

namespace qds
{

/**
 * Управляемые часы для unit-тестов.
 */
class FakeClock : public IClock
{
public:
  void set(uint64_t t);

  void advance(uint64_t dt);

  uint64_t now() const noexcept override;

private:
  uint64_t m_now = 0;
};

}