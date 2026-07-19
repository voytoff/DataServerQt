#include "packetreader.h"

#include <QtCore/qassert.h>
#include <cstring>

namespace qds
{

void PacketReader::clear()
{
  m_buffer.clear();
  m_offset = 0;
}

void PacketReader::append(const std::byte* data, std::size_t size) {
  m_buffer.insert(m_buffer.end(), data, data + size);
}

bool PacketReader::nextPacket() {

  if (m_offset != 0 && remaining() != 0)
    return false;

  if (m_offset != 0 && remaining() == 0)
    consumePacket();

  if (m_buffer.size() < HeaderSize)
    return false;

  std::memcpy(&m_header,
              m_buffer.data(),
              HeaderSize);

  if (m_header.magic != ProtocolMagic ||
      m_header.version != ProtocolVersion)
  {
    m_buffer.erase(m_buffer.begin());
    return false;
  }

  const std::size_t packetSize =
    HeaderSize + m_header.payloadSize;

  if (m_buffer.size() < packetSize)
    return false;

  m_offset = 0;

  return true;
}

bool PacketReader::readRaw(void* dst, std::size_t size)
{
  if (size > remaining())
    return false;

  const std::byte* src =
    m_buffer.data()
    + HeaderSize
    + m_offset;

  std::memcpy(dst, src, size);

  m_offset += size;

  return true;
}

PacketType PacketReader::packetType() const
{
  return m_header.type;
}

//[[nodiscard]]
size_t PacketReader::remaining() const noexcept
{
  Q_ASSERT(m_offset <= m_header.payloadSize);
  return m_header.payloadSize - m_offset;
}

size_t PacketReader::trailingBytes() const noexcept
{
  const size_t packetSize =
    HeaderSize + m_header.payloadSize;

  Q_ASSERT(m_buffer.size() >= packetSize);

  return m_buffer.size() - packetSize;
}

const PacketHeader& PacketReader::header() const noexcept
{
  return m_header;
}

void PacketReader::consumePacket()
{
  const std::size_t packetSize =
    HeaderSize + m_header.payloadSize;

  m_buffer.erase(
    m_buffer.begin(),
    m_buffer.begin() + packetSize);

  m_offset = 0;
}

}