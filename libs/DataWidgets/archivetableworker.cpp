#include "archivetableworker.h"

#include <utility>

namespace qds
{

ArchiveTableWorker::ArchiveTableWorker(
  std::filesystem::path archivePath,
  QObject* parent)
  : QObject(parent)
  , m_archivePath(std::move(archivePath))
{
}

ArchiveTableWorker::~ArchiveTableWorker() = default;

void ArchiveTableWorker::readBlock(
  uint64_t requestId,
  Timestamp experimentStart,
  uint32_t frequency,
  uint64_t firstRow,
  uint32_t rowCount,
  std::vector<SignalId> channels,
  std::shared_ptr<std::atomic_bool> cancel)
{
  if (!m_reader)
  {
    m_reader = std::make_unique<ArchiveReader>();

    if (!m_reader->open(m_archivePath))
    {
      emit blockFailed(requestId);
      return;
    }

    m_source =
      std::make_unique<ArchiveTableSource>(*m_reader);
  }

  ArchiveTableBlock block;

  const bool result = m_source->readBlock(
    experimentStart,
    frequency,
    firstRow,
    rowCount,
    channels,
    block,
    cancel.get());

  if (!result || cancel->load())
  {
    emit blockFailed(requestId);
    return;
  }

  emit blockReady(
    requestId,
    std::move(block));
}

}