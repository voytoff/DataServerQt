#pragma once

#include "archivedescriptionbuilder.h"
#include "archivedescriptionwriter.h"
#include "archiveformat.h"
#include "archivewriter.h"
#include "systemconfiguration.h"
#include <QString>
#include <filesystem>
#include <qtestcase.h>
#include <span>
#include <tuple>

namespace qds
{

struct TestArchivePoint
{
  uint64_t timestamp;
  uint64_t frameNumber;
  float value;
};

namespace fs = std::filesystem;
using ArchivePoints = std::span<TestArchivePoint>;
const fs::path archivePath = fs::path("D:/DATASERVER/archive");

class ArchiveHelper
{
public:

  static bool create(std::string dir, const ArchivePoints& points)
  {
    const auto folder = archivePath / dir;
    std::error_code ec;
    if (!fs::exists(folder, ec)) {
      // Создание каталога со всеми родительскими папками, если их нет
      fs::create_directories(folder, ec);
      if (ec) return false;
    }

    SystemConfiguration cfg;

    cfg.addCrate({.id = {0}});
    if (!cfg.addModule({.id = {0}, .crate = {0}})) return false;
    if (!cfg.addConfigurationModule({.configurationId = {0}, .module = {0}, .settings = {}})) return false;
    if (!cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}})) return false;
    if (!cfg.addConfigurationTag({.tag = {0}, .module = {0}, .channel = {0}, .settings = {}})) return false;
    if (!cfg.addSignalDefinition({.id = {10}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveRate = ArchiveRate::Hz100})) return false;

    ArchiveDescriptionBuilder descriptionBuilder;

    ArchiveDescription description;

    if (
      !descriptionBuilder.build(
        cfg,
        description)) return false;

    ArchiveDescriptionWriter descriptionWriter;

    const auto desc =
      folder / "description.json";

    const auto file =
      folder / "raw_100Hz.dat";

    if (
      !descriptionWriter.write(
        desc,
        description)) return false;

    std::array<float, 1> values;

    DataFileHeader hdr
      {
        .module = {0},
        .sampleFrequency = 100,
        .channelCount = values.size(),
        .recordSize = static_cast<uint32_t>(sizeof(SampleRecordHeader) + values.size() * sizeof(float)),
      };


    ArchiveWriter writer;
    if (!writer.open(file, hdr)) return false;
    if (!writer.isOpen()) return false;

    for (size_t n = 0; n < points.size(); n++)
    {
      const auto& xyz = points[n];
      const auto i = FrameNumber{xyz.frameNumber};
      values[0] = xyz.value;

      if (
        !writer.write(
          Timestamp{xyz.timestamp},
          i,
          WallClockTime{static_cast<int64_t>((n + 1) * 10'000)},
          values)) return false;
    }

    writer.close();

    return true;
  }
};

}