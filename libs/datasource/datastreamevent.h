#pragma once

#include "datablock.h"
#include "datastreamanchor.h"

#include <variant>

namespace qds
{

using DataStreamEvent =
  std::variant<
    DataStreamAnchor,
    DataBlock>;

}