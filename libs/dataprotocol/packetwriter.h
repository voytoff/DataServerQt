#pragma once

#include <cstddef>
#include <cstring>
#include <span>
#include <type_traits>
#include <vector>

#include "protocol/packetheader.h"
#include "protocol/packettype.h"

namespace qds
{

class PacketWriter
{
public:

  static constexpr std::size_t DefaultPacketCapacity = 4 * 1024;

public:

  PacketWriter(std::size_t capacity = DefaultPacketCapacity);

  PacketWriter(const PacketWriter&) = delete;
  PacketWriter& operator=(const PacketWriter&) = delete;

  PacketWriter(PacketWriter&&) noexcept = default;
  PacketWriter& operator=(PacketWriter&&) noexcept = default;

  void begin(PacketType type);
  void clear();

  template<typename T>
  void write(const T& value);

  template<typename T>
  void writeArray(const T* data, std::size_t count);

  void writeRaw(const void* data, std::size_t size);

  const std::byte* data() const noexcept;
  std::byte* rawData() noexcept;   // ⚠️ только для тестов
  std::size_t size() const noexcept;

  [[nodiscard]]
  std::span<const std::byte> span() const noexcept;

  void clearPayload();

private:

  PacketHeader* header() noexcept;
  const PacketHeader* header() const noexcept;

private:

  std::vector<std::byte> m_buffer;
};

//
// templates
//

template<typename T>
void PacketWriter::write(const T& value)
{
  static_assert(std::is_trivially_copyable_v<T>, "PacketWriter::write supports only trivially copyable types");
  writeRaw(&value, sizeof(T));
}

template<typename T>
void PacketWriter::writeArray(const T* data, std::size_t count)
{
  static_assert(std::is_trivially_copyable_v<T>,
                "PacketWriter::writeArray supports only trivially copyable types");

  if (!data || count == 0)
    return;

  writeRaw(data, sizeof(T) * count);
}

} // namespace qds
