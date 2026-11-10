#pragma once

#include "datatypes.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace qds
{

struct ArchiveTableBlock
{
  uint64_t firstRow = 0;
  uint32_t frequency = 100;

  std::vector<SignalId> channels;

  // values[column][row]
  // nullopt означает отсутствие измерения.
  std::vector<
    std::vector<std::optional<double>>> values;
};

}