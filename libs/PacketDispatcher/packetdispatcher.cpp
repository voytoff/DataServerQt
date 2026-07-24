#include "packetdispatcher.h"

namespace qds
{

bool qds::PacketDispatcher::dispatch(std::span<const std::byte> packet, const Endpoint &sender)
{
  return true;
}

}