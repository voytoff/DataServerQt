#ifndef ISENDER_H
#define ISENDER_H

#include <cstddef>
#include <vector>
#include "endpoint.h"

namespace qds
{

class ISender
{
public:
  virtual ~ISender() = default;

  virtual bool send(
    const Endpoint& endpoint,
    const std::byte* data,
    std::size_t size) = 0;
};

class TestSender : public ISender
{
public:
  bool send(const Endpoint&,
            const std::byte* data,
            std::size_t size) override
  {
    ++sendCount;

    packets.emplace_back(
      data,
      data + size);

    return true;
  }

  uint32_t sendCount = 0;

  std::vector<std::vector<std::byte>> packets;
};

}

#endif // ISENDER_H