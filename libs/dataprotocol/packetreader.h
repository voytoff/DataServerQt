#ifndef PACKETREADER_H
#define PACKETREADER_H

#include <vector>
#include <cstddef>

#include "protocol/packetheader.h"

namespace qds
{

class PacketReader
{
public:
  PacketReader() = default;

  void clear();
  void append(const std::byte* data, std::size_t size);

  bool nextPacket();

  const PacketHeader& header() const noexcept;

  template<typename T>
  bool read(T& value) {
    return readRaw(&value, sizeof(T));
  }

  template<typename T>
  bool readArray(T* values, std::size_t count) {
    return readRaw(values, sizeof(T) * count);
  }

  bool readRaw(void* dst, std::size_t size);

  PacketType packetType() const;

  [[nodiscard]] size_t remaining() const noexcept;
  /// Количество байтов в буфере после конца текущего пакета.
  /// Обычно равно 0.
  /// Может быть больше 0 при нескольких пакетах в одном буфере.
  [[nodiscard]] size_t trailingBytes() const noexcept;

private:
  void consumePacket();

private:
  std::vector<std::byte> m_buffer;

  PacketHeader m_header{};

  std::size_t m_offset = 0;   // позиция внутри текущего пакета

  static constexpr std::size_t HeaderSize =
    sizeof(PacketHeader);
};

}
#endif // PACKETREADER_H
