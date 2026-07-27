#include "packetreader.h"
//#include <QtCore/qassert.h>
#include <cstring>

namespace qds
{

void PacketReader::clear()
{
  m_buffer.clear();
  m_header = {};
  m_offset = 0;
}

void PacketReader::append(std::span<const std::byte> data)
{
  m_buffer.insert(
    m_buffer.end(),
    data.begin(),
    data.end());
}

void PacketReader::append(
  const std::byte* data,
  std::size_t size)
{
  append(std::span(data, size));
}

bool PacketReader::nextPacket()
{
  if (m_offset != 0)
  {
    if (remaining() != 0)
      return false;

    consumePacket();
  }

  if (m_buffer.size() < HeaderSize)
    return false;

  std::memcpy(
    &m_header,
    m_buffer.data(),
    HeaderSize);

  if (!isHeaderValid())
  {
    // Она копирует весь буфер на один байт.
    // Пока это нормально.
    // Но если когда-нибудь начнете принимать поток с большим количеством мусора перед пакетом, может оказаться выгоднее хранить индекс начала буфера и периодически выполнять компактирование.
    // Это уже микрооптимизация, и сейчас трогать её точно не стоит.
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

bool PacketReader::isHeaderValid() const
{
  return m_header.magic == ProtocolMagic &&
    m_header.version == ProtocolVersion;
}

const PacketHeader& PacketReader::header() const noexcept
{
  return m_header;
}

PacketType PacketReader::packetType() const noexcept
{
  return m_header.type;
}

std::size_t PacketReader::remaining() const noexcept
{
  //Q_ASSERT(m_offset <= m_header.payloadSize);
  assert(m_offset <= m_header.payloadSize);

  return m_header.payloadSize - m_offset;
}

std::size_t PacketReader::trailingBytes() const noexcept
{
  const std::size_t packetSize =
    HeaderSize + m_header.payloadSize;

  //Q_ASSERT(m_buffer.size() >= packetSize);
  assert(m_buffer.size() >= packetSize);

  return m_buffer.size() - packetSize;
}

const std::byte* PacketReader::payloadData() const noexcept
{
  return m_buffer.data() + HeaderSize;
}

bool PacketReader::readRaw(std::span<std::byte> dst)
{
  if (dst.size() > remaining())
    return false;

  std::memcpy(
    dst.data(),
    payloadData() + m_offset,
    dst.size());

  m_offset += dst.size();

  return true;
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