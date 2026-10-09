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

  constexpr auto operator<=>(const Sample&) const = default;
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

// Частота записи в архив, Гц.
enum class ArchiveRate : uint16_t
{
  Hz1    = 1,
  Hz10   = 10,
  Hz100  = 100,
  Hz1000 = 1000
};

constexpr bool isValidArchiveRate(ArchiveRate rate) noexcept
{
  switch (rate)
  {
  case ArchiveRate::Hz1:
  case ArchiveRate::Hz10:
  case ArchiveRate::Hz100:
  case ArchiveRate::Hz1000:
    return true;

  default:
    return false;
  }
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
