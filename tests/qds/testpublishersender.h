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
    std::span<const std::byte> data) override
  {
    if (data.empty())
      return false;

    ++sendCount;

    if (failCount > 0)
    {
      --failCount;
      return false;
    }

    m_packets.emplace_back(
      data.begin(),
      data.end());

    return true;
  }

  void clear()
  {
    m_packets.clear();
    sendCount = 0;
    failCount = 0;
  }

  const auto& lastPacket() const
  {
    return m_packets.back();
  }

public:
  std::size_t sendCount = 0;
  std::size_t failCount = 0;

  std::vector<std::vector<std::byte>>
    m_packets;
};

}