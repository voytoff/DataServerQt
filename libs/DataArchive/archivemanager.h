#pragma once

#include "archivewriter.h"

namespace qds
{

inline constexpr uint32_t BaseFrameFrequency = 1000;

constexpr bool isValidArchiveFrequency(
  uint32_t frequency) noexcept
{
  return frequency != 0 && BaseFrameFrequency % frequency == 0;
}

}