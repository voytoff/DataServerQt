#pragma once

#include <QObject>
#include <QThread>

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

class ArchiveChartController : public QObject
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

signals:
  void plotReady(
    bool success,
    qds::ArchiveSignalPoints points);

private:
  void dispatchPendingRequest();

  QThread m_thread;
  ArchiveChartWorker* m_worker = nullptr;

  uint64_t m_generation = 0;
  bool m_initialized = false;

  std::optional<ArchivePlotRequest> m_pendingRequest;
  bool m_busy = false;

  std::shared_ptr<std::atomic_bool> m_activeCancel;

};

}