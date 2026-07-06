#include "packetwriter.h"
#include "protocol/packetheader.h"
#include <qassert.h>

namespace qds
{

PacketWriter::PacketWriter()
{
  m_buffer.reserve(DefaultPacketCapacity);
}

void PacketWriter::begin(PacketType type)
{
  m_buffer.clear();
  m_buffer.resize(sizeof(PacketHeader));

  PacketHeader* h = header();

  h->magic       = ProtocolMagic;
  h->version     = ProtocolVersion;
  h->type        = type;
  h->payloadSize = 0;
}

void PacketWriter::clear()
{
  m_buffer.clear();
}

void PacketWriter::writeRaw(const void* data,
                            std::size_t size)
{
  Q_ASSERT(m_buffer.size() >= sizeof(PacketHeader));

  if (!data || size == 0)
    return;

  const std::size_t oldSize = m_buffer.size();

  m_buffer.resize(oldSize + size);

  std::memcpy(
    m_buffer.data() + oldSize,
    data,
    size
    );

  // payload size = everything after header
  header()->payloadSize =
    static_cast<uint32_t>(m_buffer.size() - sizeof(PacketHeader));
}

const std::byte* PacketWriter::data() const noexcept
{
  return reinterpret_cast<const std::byte*>(m_buffer.data());
}

std::byte *PacketWriter::rawData() noexcept
{
  return m_buffer.data();
}

std::size_t PacketWriter::size() const noexcept
{
  return m_buffer.size();
}

PacketHeader* PacketWriter::header() noexcept
{
  return reinterpret_cast<PacketHeader*>(m_buffer.data());
}

const PacketHeader* PacketWriter::header() const noexcept
{
  return reinterpret_cast<const PacketHeader*>(m_buffer.data());
}

} // namespace qds