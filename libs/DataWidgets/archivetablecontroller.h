#pragma once

#include "archivetableworker.h"
#include "datawidgets_export.h"

#include <QObject>
#include <QThread>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace qds
{

struct ArchiveBlockRequest
{
  uint64_t id = 0;
  Timestamp experimentStart;
  uint32_t frequency = 100;
  uint64_t firstRow = 0;
  uint32_t rowCount = 0;
  std::vector<SignalId> channels;
};

class DATAWIDGETS_EXPORT ArchiveTableController : public QObject
{
  Q_OBJECT

public:
  explicit ArchiveTableController(
    std::filesystem::path archivePath,
    QObject* parent = nullptr);

  ~ArchiveTableController() override;

  uint64_t readBlock(
    Timestamp experimentStart,
    uint32_t frequency,
    uint64_t firstRow,
    uint32_t rowCount,
    std::vector<SignalId> channels);

  void cancel();

signals:
  void blockReady(
    uint64_t requestId,
    qds::ArchiveTableBlock block);

  void blockFailed(
    uint64_t requestId);

private:
  void dispatchPendingRequest();
  bool finishRequest(uint64_t requestId);

  QThread m_thread;
  ArchiveTableWorker* m_worker = nullptr;
  // определяет актуальность результата.
  // при отмене или новом запросе увеличивается
  uint64_t m_generation = 0;
  std::optional<ArchiveBlockRequest> m_pendingRequest;
  bool m_busy = false;
  // определяет запрос, который действительно выполняется рабочим потоком
  uint64_t m_activeRequestId = 0;
  // Токен отмены выполняющегося запроса
  std::shared_ptr<std::atomic_bool> m_activeCancel;

};

}