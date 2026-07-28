#ifndef ARCHIVEFILE_H
#define ARCHIVEFILE_H

#include "archiveformat.h"
#include <filesystem>
#include <fstream>
#include <span>

namespace qds
{

enum class OpenMode
{
  Closed,
  Read,
  Write,
  ReadWrite
};

class ArchiveFile
{
public:
  ArchiveFile() = default;
  ~ArchiveFile();

  ArchiveFile(const ArchiveFile&) = delete;
  ArchiveFile& operator=(const ArchiveFile&) = delete;

  ArchiveFile(ArchiveFile&&) noexcept = default;
  ArchiveFile& operator=(ArchiveFile&&) noexcept = default;

  bool create(const std::filesystem::path& path,
              const DataFileHeader& header);

  bool open(const std::filesystem::path& path,
            OpenMode mode);

  void close();

  bool isOpen() const noexcept;

  [[nodiscard]]
  bool isWritable() const noexcept;

  [[nodiscard]]
  bool isReadable() const noexcept;

  [[nodiscard]]
  const std::filesystem::path& path() const noexcept;

  [[nodiscard]]
  const DataFileHeader& header() const noexcept;

  bool saveHeader();

  [[nodiscard]]
  OpenMode mode() const noexcept;

  bool write(std::span<const std::byte> data);
  bool read(std::span<std::byte> data);

  // шаблонные методы
  template<class T>
  bool writeObject(const T& value)
  {
    static_assert(std::is_trivially_copyable_v<T>);

    return write(
      std::as_bytes(
        std::span{&value, 1}));
  }
  template<class T>
  bool readObject(T& value)
  {
    static_assert(std::is_trivially_copyable_v<T>);

    return read(
      std::as_writable_bytes(
        std::span{&value, 1}));
  }

  // для массивов
  template<class T>
  bool writeArray(const T* values, std::size_t count)
  {
    static_assert(std::is_trivially_copyable_v<T>);

    if (count == 0)
      return true;

    if (!values)
      return false;

    return write(
      std::as_bytes(
        std::span{values, count}));
  }
  template<class T>
  bool readArray(T* values, std::size_t count)
  {
    static_assert(std::is_trivially_copyable_v<T>);

    if (count == 0)
      return true;

    if (!values)
      return false;

    return read(
      std::as_writable_bytes(
        std::span{values, count}));
  }

  [[nodiscard]]
  uint64_t position();

  bool seek(uint64_t pos);

  [[nodiscard]]
  uint64_t fileSize();

  [[nodiscard]]
  bool eof() const;

  bool fail() const noexcept;

  bool bad() const noexcept;

  bool good() const noexcept;

  bool flush();

  void reset();

  explicit operator bool() const noexcept;


  void setFirstTimestamp(uint64_t ts) noexcept;
  void setLastTimestamp(uint64_t ts) noexcept;

private:
  bool writeHeader() noexcept;
  bool readHeader() noexcept;

private:
  DataFileHeader m_header{};
  std::fstream m_stream;
  std::filesystem::path m_path;
  OpenMode m_mode = OpenMode::Closed;

};

}

#endif // ARCHIVEFILE_H
