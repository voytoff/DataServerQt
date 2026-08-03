#pragma once

#include <cassert>
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

  bool isHeaderValid() const;

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

  // низкоуровневый доступ
  bool readRaw(std::span<std::byte> dst);

  // копия одного объекта
  template<class T>
  bool read(T& value)
  {
    static_assert(
      std::is_trivially_copyable_v<T>);

    return readRaw(
      std::as_writable_bytes(
        std::span{&value, 1}));
  }

  // копия массива
  template<class T>
  bool readArray(T* values, std::size_t count)
  {
    static_assert(std::is_trivially_copyable_v<T>);

    if (count == 0)
      return true;

    if (!values)
      return false;

    return readRaw(
      std::as_writable_bytes(
        std::span{values, count}));
  }

  // посмотреть без чтения
  template<class T>
  [[nodiscard]]
  const T* peek() const
  {
    static_assert(std::is_trivially_copyable_v<T>);

    assert(reinterpret_cast<std::uintptr_t>(payloadData() + m_offset) % alignof(T) == 0);

    if (sizeof(T) > remaining())
      return nullptr;

    return reinterpret_cast<const T*>(
      payloadData() + m_offset);
  }

  template<class T>
  [[nodiscard]]
  bool readSpan(std::span<const T>& span, std::size_t count)
  {
    static_assert(std::is_trivially_copyable_v<T>);

    const std::size_t bytes = sizeof(T) * count;

    if (bytes > remaining())
      return false;

    span = {
      reinterpret_cast<const T*>(payloadData() + m_offset),
      count
    };

    m_offset += bytes;

    return true;
  }

  // zero-copy массив
  template<class T>
  [[nodiscard]]
  std::span<const T> readSpan(std::size_t count)
  {
    std::span<const T> span;

    if (!readSpan(span, count))
      return {};

    return span;
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