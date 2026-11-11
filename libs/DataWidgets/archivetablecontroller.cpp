#include "archivetablecontroller.h"

#include <utility>

namespace qds
{

ArchiveTableController::ArchiveTableController(
  std::filesystem::path archivePath,
  QObject* parent)
  : QObject(parent)
{
  m_worker =
    new ArchiveTableWorker(std::move(archivePath));

  m_worker->moveToThread(&m_thread);

  connect(
    m_worker,
    &ArchiveTableWorker::blockReady,
    this,
    [this](
      uint64_t requestId,
      ArchiveTableBlock block)
    {
      if (!finishRequest(requestId))
        return;

      if (requestId == m_generation)
      {
        emit blockReady(
          requestId,
          std::move(block));
      }

      dispatchPendingRequest();
    });

  connect(
    m_worker,
    &ArchiveTableWorker::blockFailed,
    this,
    [this](uint64_t requestId)
    {
      if (!finishRequest(requestId))
        return;

      if (requestId == m_generation)
        emit blockFailed(requestId);

      dispatchPendingRequest();
    });

  connect(
    &m_thread,
    &QThread::finished,
    m_worker,
    &QObject::deleteLater);

  m_thread.start();
}

ArchiveTableController::~ArchiveTableController()
{
  cancel();

  m_thread.quit();
  m_thread.wait();
}

bool ArchiveTableController::finishRequest(
  uint64_t requestId)
{
  if (!m_busy ||
      requestId != m_activeRequestId)
    return false;

  m_activeCancel.reset();
  m_activeRequestId = 0;
  m_busy = false;

  return true;
}

void ArchiveTableController::dispatchPendingRequest()
{
  if (m_busy || !m_pendingRequest)
    return;

  auto request = std::move(*m_pendingRequest);
  m_pendingRequest.reset();

  auto cancel =
    std::make_shared<std::atomic_bool>(false);

  m_activeRequestId = request.id;
  m_activeCancel =
    std::make_shared<std::atomic_bool>(false);

  m_busy = true;

  QMetaObject::invokeMethod(
    m_worker,
    [worker = m_worker,
     request = std::move(request),
     cancel]() mutable
    {
      worker->readBlock(
        request.id,
        request.experimentStart,
        request.frequency,
        request.firstRow,
        request.rowCount,
        std::move(request.channels),
        cancel);
    },
    Qt::QueuedConnection);
}

uint64_t ArchiveTableController::readBlock(
  Timestamp experimentStart,
  uint32_t frequency,
  uint64_t firstRow,
  uint32_t rowCount,
  std::vector<SignalId> channels)
{
  const uint64_t requestId = ++m_generation;

  m_pendingRequest = ArchiveBlockRequest{
    .id = requestId,
    .experimentStart = experimentStart,
    .frequency = frequency,
    .firstRow = firstRow,
    .rowCount = rowCount,
    .channels = std::move(channels)
  };

  if (m_activeCancel)
    m_activeCancel->store(true);

  dispatchPendingRequest();

  return requestId;
}

void ArchiveTableController::cancel()
{
  ++m_generation;

  m_pendingRequest.reset();

  if (m_activeCancel)
    m_activeCancel->store(true);
}

}