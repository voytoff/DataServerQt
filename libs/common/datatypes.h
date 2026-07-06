#ifndef DATATYPES_H
#define DATATYPES_H

#include <compare>
#include <cstdint>

namespace qds
{

struct TagId
{
  std::uint32_t value = 0;

  constexpr auto operator<=>(const TagId&) const = default;
};

struct ModuleId
{
  std::uint16_t value = 0;

  constexpr auto operator<=>(const ModuleId&) const = default;
};

struct CrateId
{
  std::uint16_t value = 0;

  constexpr auto operator<=>(const CrateId&) const = default;
};

struct ChannelId
{
  std::uint16_t value = 0;

  constexpr auto operator<=>(const ChannelId&) const = default;
};
}

enum class DataType {
  Double,
  Int32,
  UInt32,
  Int64,
  Bool
};

enum class Quality {
  Good,
  Bad,
  Invalid,
  Timeout
};

struct DataPoint {
  double value;
  uint64_t timestamp;
  uint32_t version;
  Quality quality;
};


#endif // DATATYPES_H
