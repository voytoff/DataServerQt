#include "archivechartworker.h"

#include <filesystem>
#include <utility>

namespace qds
{

ArchiveChartWorker::ArchiveChartWorker(
  std::filesystem::path archivePath,
  QObject* parent)
  : QObject(parent)
  , m_archivePath(std::move(archivePath))
{
}

void ArchiveChartWorker::initialize()
{
  m_source.reset();
  m_reader.reset();

  m_reader = std::make_unique<ArchiveReader>();

  if (!m_reader->open(m_archivePath))
  {
    m_reader.reset();

    emit initialized(false);
    return;
  }

  m_source =
    std::make_unique<ArchiveSignalSource>(*m_reader);

  emit initialized(true);
}

void ArchiveChartWorker::readPlot(
  uint64_t requestId,
  SignalId signalId,
  Timestamp firstTimestamp,
  Timestamp lastTimestamp,
  std::size_t maxPoints,
  std::shared_ptr<std::atomic_bool> cancel)
{
  ArchiveSignalPoints points;

  const bool success =
    m_source &&
    m_source->readPlot(
      signalId,
      firstTimestamp,
      lastTimestamp,
      maxPoints,
      points,
      cancel.get());

  emit plotReady(
    requestId,
    success,
    std::move(points));
}

}