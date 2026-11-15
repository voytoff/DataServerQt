#pragma once

#include "datatypes.h"

#include <QString>

#include <cstdint>
#include <filesystem>
#include <vector>

namespace qds
{

enum class ArchiveTableExportFormat
{
  Xlsx,
  Ods
};

struct ArchiveTableExportChannel
{
  SignalId id;
  QString name;
  QString unit;
};

struct ArchiveTableExportSettings
{
  std::filesystem::path archivePath;
  std::filesystem::path outputPath;

  ArchiveTableExportFormat format =
    ArchiveTableExportFormat::Xlsx;

  Timestamp firstTimestamp{};
  Timestamp lastTimestamp{};

  Timestamp referenceTimestamp{};
  WallClockTime referenceWallTime{};

  uint32_t frequency = 100;

  bool showUnitsInHeaders = false;

  std::vector<ArchiveTableExportChannel> channels;
};

}