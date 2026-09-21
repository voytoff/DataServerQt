#include "datastreamtime.h"
#include <cmath>

namespace qds
{

void DataStreamTime::startStream(const DataStreamAnchor& anchor)
{
  m_anchors[anchor.module] = anchor;
}

bool DataStreamTime::timestamp(
  ModuleId module,
  uint64_t frameIndex,
  Timestamp& timestamp,
  WallClockTime& wallTime) const noexcept
{
  const auto it = m_anchors.find(module);

  if (it == m_anchors.end())
    return false;

  const auto& anchor = it->second;

  if (frameIndex < anchor.firstFrameIndex)
    return false;

  if (anchor.frameRate <= 0.0)
    return false;

  const uint64_t frameOffset =
    frameIndex - anchor.firstFrameIndex;

  const double microseconds =
    static_cast<double>(frameOffset) *
    1'000'000.0 / anchor.frameRate;

  const auto delta =
    static_cast<uint64_t>(
      std::llround(microseconds));

  timestamp =
    Timestamp{
              anchor.startTimestamp.value +
              delta};

  wallTime =
    WallClockTime{
                  anchor.startWallTime.unixMicroseconds +
                  static_cast<int64_t>(delta)};

  return true;
}

}