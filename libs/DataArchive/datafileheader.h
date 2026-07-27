#pragma once

#include "datatypes.h"
#include <cstdint>

namespace qds
{

static constexpr uint32_t ArchiveMagic   = 0x44534152; // "DSAR"
static constexpr uint16_t ArchiveVersion = 1;

struct DataFileHeader
{
  uint32_t magic;          // ArchiveMagic
  uint32_t version;        // 1

  ModuleId module{};

  uint32_t channelCount = 0;
  uint32_t frequency = 0;

  uint32_t recordSize = 0;

  uint64_t startTimestamp = 0;
};

}