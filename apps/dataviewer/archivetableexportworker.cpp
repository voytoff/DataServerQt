#include "archivetableexportworker.h"
#include "archivetableexporter.h"
#include "odstablewriter.h"
#include "pathutils.h"
#include "xlsxtablewriter.h"

#include <utility>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QTemporaryDir>

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

  if (m_settings.format != ArchiveTableExportFormat::Xlsx &&
      m_settings.format != ArchiveTableExportFormat::Ods)
  {
    emit finished(false, tr("Формат пока не поддерживается"));
    return;
  }

  const QString output =
    QDir::fromNativeSeparators(
      pathToQString(
        m_settings.outputPath.wstring()));

  const QFileInfo outputInfo(output);

  if (!outputInfo.dir().exists())
  {
    emit finished(false, tr("Каталог назначения не существует"));
    return;
  }

  const QString extension =
    m_settings.format == ArchiveTableExportFormat::Ods
      ? QStringLiteral("ods")
      : QStringLiteral("xlsx");

  QTemporaryDir temporaryDir(
    outputInfo.dir().filePath(
      QStringLiteral(".dataexport-XXXXXX")));

  if (!temporaryDir.isValid())
  {
    emit finished(
      false,
      tr("Не удалось создать временный каталог: %1")
        .arg(temporaryDir.errorString()));
    return;
  }

  const QString temporaryName =
    temporaryDir.filePath(
      QStringLiteral("table.%1").arg(extension));

  ArchiveTableExportSettings settings = m_settings;
  settings.outputPath =
    std::filesystem::path(temporaryName.toStdWString());

  std::unique_ptr<ArchiveTableWriter> writer;

  switch (settings.format)
  {
  case ArchiveTableExportFormat::Xlsx:
    writer = std::make_unique<XlsxTableWriter>(
      settings.outputPath);
    break;

  case ArchiveTableExportFormat::Ods:
    writer = std::make_unique<OdsTableWriter>(
      settings.outputPath);
    break;

  default:
    emit finished(false, tr("Формат пока не поддерживается"));
    return;
  }

  ArchiveTableExporter exporter;

  const bool success =
    exporter.exportTable(
      settings,
      *writer,
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
      tr("Не удалось сохранить %1: %2")
        .arg(extension.toUpper(), error));
    return;
  }

  emit finished(true, {});
}

}
