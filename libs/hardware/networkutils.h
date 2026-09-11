#pragma once

#include <cstdint>
#include <string_view>

namespace qds
{
// ""            -> LTRD_ADDR_DEFAULT, true
// "127.0.0.1"   -> parsed IPv4, true
// invalid IPv4  -> false
bool parseIpv4Address(
  std::string_view address,
  uint32_t& result) noexcept;

}