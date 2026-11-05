#include "archivechartcontroller.h"

#include <QMetaObject>

#include <utility>

namespace qds
{

ArchiveChartController::ArchiveChartController(
  std::filesystem::path archivePath,
  QObject* parent)
  : QObject(parent)
{
  m_worker =
    new ArchiveChartWorker(std::move(archivePath));

  m_worker->moveToThread(&m_thread);

  connect(
    &m_thread,
    &QThread::started,
    m_worker,
    &ArchiveChartWorker::initialize);

  connect(
    m_worker,
    &ArchiveChartWorker::initialized,
    this,
    [this](bool success)
    {
      m_initialized = success;

      if (success)
        dispatchPendingRequest();

      if (success && m_pendingTimeRangeSignal)
      {
        const auto signalId = *m_pendingTimeRangeSignal;
        requestTimeRange(signalId);
      }
    });

  connect(
    m_worker,
    &ArchiveChartWorker::plotReady,
    this,
    [this](
      uint64_t requestId,
      bool success,
      ArchiveSignalSegments segments)
    {
      m_busy = false;
      m_activeCancel.reset();

      if (requestId == m_generation)
      {
        emit plotReady(
          success,
          std::move(segments));
      }

      dispatchPendingRequest();
    });

  connect(
    m_worker,
    &ArchiveChartWorker::timeRangeReady,
    this,
    [this](
      uint64_t requestId,
      bool success,
      Timestamp firstTimestamp,
      Timestamp lastTimestamp)
    {
      if (requestId != m_timeRangeGeneration)
        return;

      emit timeRangeReady(
        success,
        firstTimestamp,
        lastTimestamp);
    });

  connect(
    &m_thread,
    &QThread::finished,
    m_worker,
    &QObject::deleteLater);

  m_thread.start();
}

ArchiveChartController::~ArchiveChartController()
{
  if (m_activeCancel)
    m_activeCancel->store(true, std::memory_order_relaxed);

  m_thread.quit();
  m_thread.wait();
}

void ArchiveChartController::requestPlot(
  SignalId signalId,
  Timestamp firstTimestamp,
  Timestamp lastTimestamp,
  std::size_t maxPoints)
{
  if (m_activeCancel)
    m_activeCancel->store(true, std::memory_order_relaxed);

  m_pendingRequest = ArchivePlotRequest{
    .id = ++m_generation,
    .signalId = signalId,
    .firstTimestamp = firstTimestamp,
    .lastTimestamp = lastTimestamp,
    .maxPoints = maxPoints
  };

  dispatchPendingRequest();
}

void ArchiveChartController::requestTimeRange(
  SignalId signalId)
{
  const auto requestId = ++m_timeRangeGeneration;

  m_pendingTimeRangeSignal = signalId;

  if (!m_initialized)
    return;

  m_pendingTimeRangeSignal.reset();

  QMetaObject::invokeMethod(
    m_worker,
    [worker = m_worker, requestId, signalId]()
    {
      worker->readTimeRange(requestId, signalId);
    },
    Qt::QueuedConnection);
}

void ArchiveChartController::dispatchPendingRequest()
{
  if (!m_initialized || m_busy || !m_pendingRequest)
    return;

  const auto request = *m_pendingRequest;
  m_pendingRequest.reset();

  m_activeCancel =
    std::make_shared<std::atomic_bool>(false);

  auto cancel = m_activeCancel;

  m_busy = true;

  QMetaObject::invokeMethod(
    m_worker,
    [worker = m_worker, request, cancel]()
    {
      worker->readPlot(
        request.id,
        request.signalId,
        request.firstTimestamp,
        request.lastTimestamp,
        request.maxPoints,
        cancel);
    },
    Qt::QueuedConnection);
}

}