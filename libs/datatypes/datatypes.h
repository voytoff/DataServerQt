#ifndef DATATYPES_H
#define DATATYPES_H

#include <compare>
#include <cstdint>
#include <string>

namespace qds
{

struct TagId
{
  uint32_t value = 0;
  constexpr auto operator<=>(const TagId&) const = default;
};

struct ModuleId
{
  uint32_t value = 0;
  constexpr auto operator<=>(const ModuleId&) const = default;
};

struct CrateId
{
  uint32_t value = 0;
  constexpr auto operator<=>(const CrateId&) const = default;
};

struct ChannelId
{
  uint32_t value = 0;
  constexpr auto operator<=>(const ChannelId&) const = default;
};

struct SubscriptionId
{
  uint32_t value = 0;
  constexpr auto operator<=>(const SubscriptionId&) const = default;
};

struct Sample
{
  float value = 0.0f;
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

}

#endif // DATATYPES_H
