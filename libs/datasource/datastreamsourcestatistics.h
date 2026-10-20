#pragma once

#include <cstdint>

namespace qds
{

struct DataStreamSourceStatistics
{
  uint64_t blocksReceived = 0;
  uint64_t framesReceived = 0;
  uint64_t lastTimestamp = 0;
};

}