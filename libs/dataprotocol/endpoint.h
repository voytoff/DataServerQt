#pragma once

#include <cstdint>
#include <string>

namespace qds
{

struct Endpoint
{
  std::string address;
  uint16_t port = 0;

  auto operator<=>(const Endpoint&) const = default;

  [[nodiscard]]
  std::string toString() const;
};

}