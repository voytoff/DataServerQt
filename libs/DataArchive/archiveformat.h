#pragma once

#include <cstdint>

#include "datatypes.h"

namespace qds
{

inline constexpr uint32_t ArchiveMagic   = 0x51445341; // "QDSA"
inline constexpr uint32_t ArchiveVersion = 1;

struct SampleRecordHeader
{
  uint64_t timestamp;          // monotonic, µs
  uint64_t frameNumber;        // номер Frame
  int64_t  wallTime;           // Unix epoch, µs
};

static_assert(
  std::is_trivially_copyable_v<SampleRecordHeader>);

static_assert(
  sizeof(SampleRecordHeader) == 24);

enum class ArchiveDataType : uint32_t
{
  Unknown = 0,
  RawModule,
  Formula,
  Calculated,
  User
};

struct DataFileHeader
{
  uint32_t magic = ArchiveMagic;
  uint32_t version = ArchiveVersion;

  ModuleId module{};

  uint32_t sampleFrequency = 0;
  uint32_t channelCount = 0;
  uint32_t recordSize = 0;

  uint64_t startTimestamp = 0;   // время создания файла

  uint64_t firstTimestamp = 0;   // время первой записанной выборки
  uint64_t lastTimestamp  = 0;   // время последней записанной выборки

  uint64_t recordCount = 0;

  uint32_t headerSize = uint32_t(sizeof(DataFileHeader));

  uint32_t reserved[8]{};

  constexpr bool isValid() const noexcept
  {
    return magic == ArchiveMagic
           && version == ArchiveVersion
           && headerSize >= sizeof(DataFileHeader)
           && channelCount > 0
           && recordSize ==
                sizeof(SampleRecordHeader) +
                  channelCount * sizeof(float);
  }

};

static constexpr uint64_t HeaderSize =
  sizeof(DataFileHeader);

static_assert(std::is_trivially_copyable_v<DataFileHeader>);
static_assert(sizeof(DataFileHeader) % 8 == 0);

}
