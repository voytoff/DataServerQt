#ifndef ARCHIVEFILE_H
#define ARCHIVEFILE_H

#include <fstream>
#include <span>

namespace qds
{

class ArchiveFile
{
public:
  ArchiveFile() = default;

  bool openRead(const std::filesystem::path& file);
  bool openWrite(const std::filesystem::path& file);

  bool close();

  bool isOpen() const noexcept;

  bool flush();

  bool seek(uint64_t position);
  uint64_t position() const;

  bool write(std::span<const std::byte> data);
  bool read(std::span<std::byte> data);

  uint64_t size() const;

private:
  std::fstream m_stream;
};

}

#endif // ARCHIVEFILE_H
