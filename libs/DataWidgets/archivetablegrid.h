#pragma once

#include "datatypes.h"
#include "timeconstants.h"

#include <cstdint>

namespace qds
{

struct ArchiveTableGrid
{
  uint64_t start = 0;
  uint64_t end = 0;

  uint32_t frequency = 100;

  int64_t wallOrigin = 0;

  [[nodiscard]]
  static ArchiveTableGrid create(
    Timestamp firstTimestamp,
    Timestamp lastTimestamp,
    Timestamp referenceTimestamp,
    WallClockTime referenceWallTime,
    uint32_t frequency) noexcept
  {
    ArchiveTableGrid grid;

    grid.start = firstTimestamp.value;
    grid.end = lastTimestamp.value;
    grid.frequency = frequency;

    const int64_t offset =
      static_cast<int64_t>(
        firstTimestamp.value -
        referenceTimestamp.value);

    const int64_t firstWallTime =
      referenceWallTime.unixMicroseconds + offset;

    // Округление к ближайшей секунде.
    const int64_t seconds =
      firstWallTime / MicrosecondsPerSecond;

    const int64_t remainder =
      firstWallTime % MicrosecondsPerSecond;

    grid.wallOrigin =
      (seconds + (remainder >= 500'000 ? 1 : 0)) *
      MicrosecondsPerSecond;

    return grid;
  }

  [[nodiscard]]
  uint64_t step() const noexcept
  {
    return MicrosecondsPerSecond / frequency;
  }

  [[nodiscard]]
  uint64_t rowCount() const noexcept
  {
    if (end < start)
      return 0;

    return (end - start) / step() + 1;
  }

  [[nodiscard]]
  uint64_t offset(uint64_t row) const noexcept
  {
    return row * step();
  }

  [[nodiscard]]
  double seconds(uint64_t row) const noexcept
  {
    return static_cast<double>(offset(row)) /
           MicrosecondsPerSecond;
  }

  [[nodiscard]]
  int64_t wallMilliseconds(uint64_t row) const noexcept
  {
    return wallOrigin / 1000 +
           static_cast<int64_t>(offset(row) / 1000);
  }
};

}