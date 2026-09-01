#pragma once

#include "isender.h"
#include <vector>

namespace qds
{

class TestPublisherSender : public ISender
{
public:
  bool send(
    const Endpoint&,
    std::span<const std::byte> data) override;

  void clear();

  const auto& lastPacket() const;

public:
  std::size_t sendCount = 0;
  std::vector<std::vector<std::byte>> m_packets;
};

}