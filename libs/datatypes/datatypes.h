#pragma once

#include <compare>
#include <cstdint>
#include "strongid.h"

namespace qds
{

static constexpr uint32_t InvalidIndex32 = UINT32_MAX;

template<typename Tag>
constexpr StrongId<Tag> InvalidId()
{
  return {UINT32_MAX};
}

struct TagIdTag {};
struct ModuleIdTag {};
struct CrateIdTag {};
struct ChannelIdTag {};
struct SubscriptionIdTag {};

using TagId =
  StrongId<TagIdTag>;

using ModuleId =
  StrongId<ModuleIdTag>;

using CrateId =
  StrongId<CrateIdTag>;

using ChannelId =
  StrongId<ChannelIdTag>;

using SubscriptionId =
  StrongId<SubscriptionIdTag>;

struct Sample
{
  double value = 0.0;
};

enum class PublishRate : uint16_t
{
  Hz1   = 1,
  Hz10  = 10,
  Hz100 = 100
};

constexpr uint32_t toHz(PublishRate r)
{
  return static_cast<uint32_t>(r);
}

struct Timestamp
{
  // Внутренняя монотонная шкала времени, микросекунды
  uint64_t value = 0;
  constexpr auto operator<=>(const Timestamp&) const = default;
};

struct WallClockTime
{
  // Реальное календарное время Unix epoch, микросекунды
  int64_t unixMicroseconds = 0;
  constexpr auto operator<=>(const WallClockTime&) const = default;
};

struct FrameNumber
{
  uint64_t value = 0;
  constexpr auto operator<=>(const FrameNumber&) const = default;
};

}
