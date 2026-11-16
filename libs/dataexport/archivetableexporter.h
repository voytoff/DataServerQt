#pragma once

#include "archivetableexportsettings.h"
#include "archivetablewriter.h"
#include "dataexport_global.h"

#include <QString>

#include <atomic>
#include <cstdint>
#include <functional>

namespace qds
{

using StageCallback =
  std::function<void()>;

class DATAEXPORT_EXPORT ArchiveTableExporter
{
public:
  using ProgressCallback =
    std::function<void(uint64_t, uint64_t)>;

  bool exportTable(
    const ArchiveTableExportSettings& settings,
    ArchiveTableWriter& writer,
    const std::atomic_bool* cancel = nullptr,
    ProgressCallback progress = {},
    StageCallback finalizing = {});

  [[nodiscard]]
  QString errorString() const;

private:
  QString m_error;
};

}