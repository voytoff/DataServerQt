#pragma once

#include "datastreamanchor.h"
#include <strongidhash.h>
#include <unordered_map>

namespace qds
{

class DataStreamTime
{
public:
  void startStream(
    const DataStreamAnchor& anchor);

  [[nodiscard]]
  bool timestamp(
    ModuleId module,
    uint64_t frameIndex,
    Timestamp& timestamp,
    WallClockTime& wallTime) const noexcept;

private:
  std::unordered_map<
    ModuleId,
    DataStreamAnchor> m_anchors;
};

}