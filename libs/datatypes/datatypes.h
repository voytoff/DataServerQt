#pragma once

#include <compare>
#include <cstdint>
#include <strongid.h>

namespace qds
{

static constexpr uint32_t InvalidIndex32 = UINT32_MAX;

template<typename Tag>
constexpr StrongId<Tag> InvalidId()
{
  return {UINT32_MAX};
}

struct SignalIdTag {};
struct TagIdTag {};
struct ModuleIdTag {};
struct CrateIdTag {};
struct ChannelIdTag {};
struct SubscriptionIdTag {};
struct SignalTypeIdTag {};
struct ConfigurationIdTag {};

using SignalId =
  StrongId<SignalIdTag>;

static constexpr SignalId InvalidSignalId{UINT32_MAX};

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

using SignalTypeId =
  StrongId<SignalTypeIdTag>;

using ConfigurationId =
  StrongId<ConfigurationIdTag>;

struct Sample
{
  double value = 0.0;
};

// Частота публикации данных для подписки.
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
