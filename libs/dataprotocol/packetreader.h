#ifndef PACKETREADER_H
#define PACKETREADER_H

#include <cstddef>
#include <span>
#include <vector>

#include "protocol/packetheader.h"

namespace qds
{

class PacketReader
{
public:
  PacketReader() = default;

  void clear();

  void append(std::span<const std::byte> data);

  void append(const std::byte* data, std::size_t size);

  [[nodiscard]]
  bool nextPacket();

  [[nodiscard]]
  const PacketHeader& header() const noexcept;

  [[nodiscard]]
  PacketType packetType() const noexcept;

  [[nodiscard]]
  std::size_t remaining() const noexcept;

  [[nodiscard]]
  std::size_t trailingBytes() const noexcept;

  [[nodiscard]]
  const std::byte* payloadData() const noexcept;

  bool readBytes(std::span<std::byte> dst);

  template<class T>
  bool read(T& value)
  {
    return readBytes(
      std::as_writable_bytes(
        std::span{&value, 1}));
  }

  template<class T>
  bool readArray(T* values, std::size_t count)
  {
    return readBytes(
      std::as_writable_bytes(
        std::span{values, count}));
  }

private:
  void consumePacket();

private:
  std::vector<std::byte> m_buffer;

  PacketHeader m_header{};

  std::size_t m_offset = 0;

  static constexpr std::size_t HeaderSize =
    sizeof(PacketHeader);
};

}

#endif // PACKETREADER_H