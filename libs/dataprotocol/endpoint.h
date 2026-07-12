#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <cstdint>
#include <string>

namespace qds
{

struct Endpoint
{
  std::string address;
  uint16_t port = 0;

  auto operator<=>(const Endpoint&) const = default;
  //mutable std::optional<QHostAddress> cache;
};

}

#endif // ENDPOINT_H
