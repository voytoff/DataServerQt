#pragma once

#include "endpoint.h"
#include <cstddef>
#include <span>
#include <vector>

namespace qds
{

class ISender
{
public:
  virtual ~ISender() = default;

  virtual bool send(
    const Endpoint& endpoint,
    std::span<const std::byte> data) = 0;
};

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

    m_packets.emplace_back(
      data.begin(),
      data.end());

    return true;
  }

  void clear()
  {
    m_packets.clear();
    sendCount = 0;
  }

  const auto& lastPacket() const
  {
    return m_packets.back();
  }

public:
  std::size_t sendCount = 0;
  std::vector<std::vector<std::byte>> m_packets;
};

}