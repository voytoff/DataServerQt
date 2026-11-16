
#pragma once

#include "archivetableexportsettings.h"

#include <QObject>
#include <QString>

#include <atomic>
#include <cstdint>
#include <memory>

namespace qds
{

class ArchiveTableExportWorker final : public QObject
{
  Q_OBJECT

public:
  explicit ArchiveTableExportWorker(
    ArchiveTableExportSettings settings,
    std::shared_ptr<std::atomic_bool> cancel);

public slots:
  void run();

signals:
  void progressChanged(
    uint64_t completed,
    uint64_t total);

  void finished(
    bool success,
    const QString& error);

private:
  ArchiveTableExportSettings m_settings;
  std::shared_ptr<std::atomic_bool> m_cancel;
};

}
