#ifndef DATATYPES_H
#define DATATYPES_H

#include <compare>
#include <cstdint>

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
  auto operator<=>(const SubscriptionId&) const = default;
};

struct Sample
{
  float value = 0.0f;
  uint32_t quality = 0;
};

//struct TagRecord
//{
//  ModuleId module;
//  ChannelId channel;
//};
struct TagRecord
{
  uint64_t timestamp = 0;
  float value = 0.0f;
};

}

#endif // DATATYPES_H
