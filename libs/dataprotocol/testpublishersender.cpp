#include "testpublishersender.h"

namespace qds
{

bool qds::TestPublisherSender::send(
  const Endpoint &,
  std::span<const std::byte> data)
{
  if (data.empty())
    return false;

  ++sendCount;

  m_packets.emplace_back(
    data.begin(),
    data.end());

  return true;
}

void TestPublisherSender::clear()
{
  m_packets.clear();
  sendCount = 0;
}

const auto &TestPublisherSender::lastPacket() const
{
  return m_packets.back();
}

}
