#include "archivetableexportworker.h"
#include "archivetableexporter.h"
#include "xlsxtablewriter.h"

#include <utility>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QSaveFile>

namespace qds
{

namespace
{

bool commitExportFile(
  const QString& temporaryName,
  const QString& outputName,
  QString& error)
{
  QFile source(temporaryName);

  if (!source.open(QIODevice::ReadOnly))
  {
    error = source.errorString();
    return false;
  }

  QSaveFile target(outputName);

  if (!target.open(QIODevice::WriteOnly))
  {
    error = target.errorString();
    return false;
  }

  constexpr qint64 BufferSize = 1024 * 1024;

  QByteArray buffer;
  buffer.resize(BufferSize);

  while (true)
  {
    const qint64 read =
      source.read(buffer.data(), buffer.size());

    if (read < 0)
    {
      error = source.errorString();
      target.cancelWriting();
      return false;
    }

    if (read == 0)
      break;

    if (target.write(buffer.constData(), read) != read)
    {
      error = target.errorString();
      target.cancelWriting();
      return false;
    }
  }

  if (!target.commit())
  {
    error = target.errorString();
    return false;
  }

  return true;
}

}

ArchiveTableExportWorker::ArchiveTableExportWorker(
  ArchiveTableExportSettings settings,
  std::shared_ptr<std::atomic_bool> cancel)
  : m_settings(std::move(settings))
  , m_cancel(std::move(cancel))
{
}

void ArchiveTableExportWorker::run()
{
  if (m_cancel->load())
  {
    emit finished(false, tr("Экспорт отменён"));
    return;
  }

  if (m_settings.format != ArchiveTableExportFormat::Xlsx)
  {
    emit finished(false, tr("Формат пока не поддерживается"));
    return;
  }

  const QString output =
    QDir::fromNativeSeparators(
      QString::fromStdWString(
        m_settings.outputPath.wstring()));

  const QFileInfo outputInfo(output);

  if (!outputInfo.dir().exists())
  {
    emit finished(false, tr("Каталог назначения не существует"));
    return;
  }

  QTemporaryFile temporary(
    outputInfo.dir().filePath(
      QStringLiteral(".dataexport-XXXXXX.xlsx")));

  if (!temporary.open())
  {
    emit finished(false, temporary.errorString());
    return;
  }

  const QString temporaryName = temporary.fileName();

  // libxlsxwriter открывает файл самостоятельно.
  temporary.close();

  ArchiveTableExportSettings settings = m_settings;
  settings.outputPath =
    std::filesystem::path(temporaryName.toStdWString());

  XlsxTableWriter writer(settings.outputPath);
  ArchiveTableExporter exporter;

  const bool success =
    exporter.exportTable(
      settings,
      writer,
      m_cancel.get(),
      [this](uint64_t completed, uint64_t total)
      {
        emit progressChanged(completed, total);
      },
      [this]()
      {
        emit stageChanged(
          tr("Завершение экспорта…"));
      });

  if (!success)
  {
    emit finished(false, exporter.errorString());
    return;
  }

  if (m_cancel->load())
  {
    emit finished(false, tr("Экспорт отменён"));
    return;
  }

  QString error;

  if (!commitExportFile(
        temporaryName,
        output,
        error))
  {
    emit finished(
      false,
      tr("Не удалось сохранить XLSX: %1")
        .arg(error));
    return;
  }

  emit finished(true, {});
}

}
