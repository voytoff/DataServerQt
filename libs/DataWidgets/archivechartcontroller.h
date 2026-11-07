#pragma once

#include <QObject>
#include <QThread>

#include "datawidgets_export.h"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <atomic>
#include <memory>

#include "archivechartworker.h"

namespace qds
{

struct ArchivePlotRequest
{
  uint64_t id = 0;
  SignalId signalId;
  Timestamp firstTimestamp;
  Timestamp lastTimestamp;
  std::size_t maxPoints = 0;
};

class DATAWIDGETS_EXPORT ArchiveChartController : public QObject
{
  Q_OBJECT

public:
  explicit ArchiveChartController(
    std::filesystem::path archivePath,
    QObject* parent = nullptr);

  ~ArchiveChartController() override;

  void requestPlot(
    SignalId signalId,
    Timestamp firstTimestamp,
    Timestamp lastTimestamp,
    std::size_t maxPoints);

  void requestTimeRange(
    SignalId signalId);

private:
  void dispatchPendingRequest();

  QThread m_thread;
  ArchiveChartWorker* m_worker = nullptr;

  uint64_t m_generation = 0;
  bool m_initialized = false;

  std::optional<ArchivePlotRequest> m_pendingRequest;
  bool m_busy = false;

  std::shared_ptr<std::atomic_bool> m_activeCancel;

  std::optional<SignalId> m_pendingTimeRangeSignal;
  uint64_t m_timeRangeGeneration = 0;

signals:
  void timeRangeReady(
    bool success,
    qds::Timestamp firstTimestamp,
    qds::Timestamp lastTimestamp);

  void plotReady(
    bool success,
    qds::ArchiveSignalSegments segments);

};

}