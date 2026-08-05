#pragma once

#include <compare>
#include <cstdint>

namespace qds
{

template<typename Tag>
struct StrongId
{
  uint32_t value = 0;

  constexpr auto operator<=>(const StrongId&) const = default;

  constexpr explicit operator bool() const noexcept
  {
    return value != 0;
  }
};

}