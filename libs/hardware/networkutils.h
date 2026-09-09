#pragma once

#include <cstdint>
#include <string_view>

namespace qds
{

bool parseIpv4Address(
  std::string_view address,
  uint32_t& result) noexcept;

}