#pragma once

#include "archivereader.h"
#include "archivetableblock.h"
#include "archivetablesource.h"
#include "datawidgets_export.h"

#include <QObject>

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

namespace qds
{

class DATAWIDGETS_EXPORT ArchiveTableWorker : public QObject
{
  Q_OBJECT

public:
  explicit ArchiveTableWorker(
    std::filesystem::path archivePath,
    QObject* parent = nullptr);

  ~ArchiveTableWorker() override;

public slots:
  void readBlock(
    uint64_t requestId,
    qds::Timestamp experimentStart,
    uint32_t frequency,
    uint64_t firstRow,
    uint32_t rowCount,
    std::vector<qds::SignalId> channels,
    std::shared_ptr<std::atomic_bool> cancel);

signals:
  void blockReady(
    uint64_t requestId,
    qds::ArchiveTableBlock block);

  void blockFailed(
    uint64_t requestId);

private:
  std::filesystem::path m_archivePath;

  std::unique_ptr<ArchiveReader> m_reader;
  std::unique_ptr<ArchiveTableSource> m_source;

};

}