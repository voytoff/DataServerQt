#include "packetreader.h"

#include <cstring>

namespace qds
{

// -----------------------------
// clear
// -----------------------------
void PacketReader::clear()
{
  m_buffer.clear();
  m_offset = 0;
}

// -----------------------------
// append
// -----------------------------
void PacketReader::append(const std::byte* data, std::size_t size)
{
  m_buffer.insert(m_buffer.end(), data, data + size);
}

// -----------------------------
// nextPacket
// -----------------------------
bool PacketReader::nextPacket()
{
  if (m_offset != 0 && !eof())
    return false;

  if (m_offset != 0 && eof())
    consumePacket();

  if (m_buffer.size() < sizeof(PacketHeader))
    return false;

  std::memcpy(&m_header,
              m_buffer.data(),
              sizeof(PacketHeader));

  // ❗ TEST4 FIX
  if (m_header.magic != ProtocolMagic)
  {
    m_buffer.erase(m_buffer.begin());
    return false;
  }

  const std::size_t packetSize =
    sizeof(PacketHeader) + m_header.payloadSize;

  if (m_buffer.size() < packetSize)
    return false;

  m_offset = 0;

  return true;
}

// -----------------------------
// readRaw
// -----------------------------
bool PacketReader::readRaw(void* dst, std::size_t size)
{
  const std::size_t packetSize =
    sizeof(PacketHeader) + m_header.payloadSize;

  if (m_offset + size > packetSize)
    return false;

  const std::byte* src =
    m_buffer.data() + sizeof(PacketHeader) + m_offset;

  std::memcpy(dst, src, size);

  m_offset += size;

  return true;
}

// -----------------------------
// eof
// -----------------------------
bool PacketReader::eof() const noexcept
{
  return m_offset == m_header.payloadSize;
}

// -----------------------------
// bytesRemaining
// -----------------------------
std::size_t PacketReader::bytesRemaining() const noexcept
{
  return m_header.payloadSize - m_offset;
}

// -----------------------------
// header
// -----------------------------
const PacketHeader& PacketReader::header() const noexcept
{
  return m_header;
}

// -----------------------------
// consumePacket
// -----------------------------
void PacketReader::consumePacket()
{
  const std::size_t packetSize =
    sizeof(PacketHeader) + m_header.payloadSize;

  m_buffer.erase(
    m_buffer.begin(),
    m_buffer.begin() + packetSize);

  m_offset = 0;
}

}