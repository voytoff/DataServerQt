#pragma once

#include "idatastreamsource.h"
#include <cstdint>

namespace qds
{

class TestDataStreamSource final
  : public IDataStreamSource
{
public:
  TestDataStreamSource(const int32_t startCount = -1)
    : m_startCount(startCount) {}

  bool start() noexcept override
  {
    ++startCounts;

    if (m_startCount == 0)
    {
      running = false;
      return false;
    }

    if (m_startCount > 0)
      --m_startCount;

    running = true;
    return true;
  }

  void stop() noexcept override
  {
    ++stopCounts;
    running = false;
  }

  int32_t startCounts = 0;
  int32_t stopCounts = 0;

private:
  int32_t m_startCount;

  bool running = false;
};

}