#pragma once

#include <QObject>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <atomic>
#include <memory>

#include "archivereader.h"
#include "archivesignalsource.h"

namespace qds
{

class ArchiveChartWorker : public QObject
{
  Q_OBJECT

public:
  explicit ArchiveChartWorker(
    std::filesystem::path archivePath,
    QObject* parent = nullptr);

public slots:
  void initialize();

  void readPlot(
    uint64_t requestId,
    qds::SignalId signalId,
    qds::Timestamp firstTimestamp,
    qds::Timestamp lastTimestamp,
    std::size_t maxPoints,
    std::shared_ptr<std::atomic_bool> cancel);

  void readTimeRange(
    uint64_t requestId,
    qds::SignalId signalId);

signals:
  void initialized(bool success);

  void plotReady(
    uint64_t requestId,
    bool success,
    qds::ArchiveSignalSegments segments);

  void timeRangeReady(
    uint64_t requestId,
    bool success,
    qds::Timestamp firstTimestamp,
    qds::Timestamp lastTimestamp);

private:
  std::filesystem::path m_archivePath;

  std::unique_ptr<ArchiveReader> m_reader;
  std::unique_ptr<ArchiveSignalSource> m_source;

  std::shared_ptr<std::atomic_bool> m_cancel;
};

}