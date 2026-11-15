
#include "archivetableexporter.h"

#include "archivereader.h"
#include "archivetablegrid.h"
#include "archivetablesource.h"

#include <algorithm>
#include <optional>
#include <utility>
#include <vector>

namespace qds
{

bool ArchiveTableExporter::exportTable(
  const ArchiveTableExportSettings& settings,
  ArchiveTableWriter& writer,
  const std::atomic_bool* cancel,
  ProgressCallback progress)
{
  m_error.clear();

  constexpr uint32_t BlockSize = 4096;

  const auto cancelled = [cancel]() noexcept
  {
    return cancel && cancel->load();
  };

  if (cancelled())
  {
    m_error = QStringLiteral("Экспорт отменён");
    return false;
  }

  if (settings.frequency != 1 &&
      settings.frequency != 10 &&
      settings.frequency != 100 &&
      settings.frequency != 1000)
  {
    m_error = QStringLiteral(
      "Недопустимая частота экспорта");
    return false;
  }

  if (settings.lastTimestamp.value <
      settings.firstTimestamp.value)
  {
    m_error = QStringLiteral(
      "Недопустимый временной диапазон");
    return false;
  }

  ArchiveReader reader;

  if (!reader.open(settings.archivePath))
  {
    m_error = QStringLiteral(
      "Не удалось открыть архив");
    return false;
  }

  ArchiveTableSource source(reader);

  const ArchiveTableGrid grid =
    ArchiveTableGrid::create(
      settings.firstTimestamp,
      settings.lastTimestamp,
      settings.referenceTimestamp,
      settings.referenceWallTime,
      settings.frequency);

  const uint64_t totalRows = grid.rowCount();

  if (totalRows == 0)
  {
    m_error = QStringLiteral(
      "Нет данных для экспорта");
    return false;
  }

  std::vector<SignalId> signalIds;
  std::vector<QString> headers;

  signalIds.reserve(settings.channels.size());
  headers.reserve(settings.channels.size() + 2);

  headers.emplace_back(QStringLiteral("Секунды"));
  headers.emplace_back(QStringLiteral("Время"));

  for (const auto& channel : settings.channels)
  {
    signalIds.push_back(channel.id);

    QString header = channel.name;

    if (settings.showUnitsInHeaders &&
        !channel.unit.isEmpty())
    {
      header += QStringLiteral(", ") + channel.unit;
    }

    headers.push_back(std::move(header));
  }

  if (!writer.begin(headers))
  {
    m_error = writer.errorString();
    writer.abort();
    return false;
  }

  std::vector<std::optional<double>> rowValues(
    signalIds.size());

  uint64_t firstRow = 0;

  while (firstRow < totalRows)
  {
    if (cancelled())
    {
      m_error = QStringLiteral("Экспорт отменён");
      writer.abort();
      return false;
    }

    const uint32_t rowCount =
      static_cast<uint32_t>(
        std::min<uint64_t>(
          BlockSize,
          totalRows - firstRow));

    ArchiveTableBlock block;

    if (!source.readBlock(
          settings.firstTimestamp,
          settings.frequency,
          firstRow,
          rowCount,
          signalIds,
          block,
          cancel))
    {
      m_error = cancelled()
      ? QStringLiteral("Экспорт отменён")
      : QStringLiteral(
          "Ошибка чтения данных архива");

      writer.abort();
      return false;
    }

    if (block.values.size() != signalIds.size())
    {
      m_error = QStringLiteral(
        "Некорректный блок данных архива");

      writer.abort();
      return false;
    }

    for (const auto& column : block.values)
    {
      if (column.size() != rowCount)
      {
        m_error = QStringLiteral(
          "Некорректный размер блока данных");

        writer.abort();
        return false;
      }
    }

    for (uint32_t row = 0; row < rowCount; ++row)
    {
      if (cancelled())
      {
        m_error = QStringLiteral("Экспорт отменён");
        writer.abort();
        return false;
      }

      for (std::size_t column = 0;
           column < block.values.size();
           ++column)
      {
        rowValues[column] = block.values[column][row];
      }

      const uint64_t globalRow = firstRow + row;

      if (!writer.writeRow(
            grid.seconds(globalRow),
            grid.wallMilliseconds(globalRow),
            rowValues))
      {
        m_error = writer.errorString();
        writer.abort();
        return false;
      }
    }

    firstRow += rowCount;

    if (progress)
      progress(firstRow, totalRows);
  }

  if (cancelled())
  {
    m_error = QStringLiteral("Экспорт отменён");
    writer.abort();
    return false;
  }

  if (!writer.finish())
  {
    m_error = writer.errorString();
    writer.abort();
    return false;
  }

  return true;
}

QString ArchiveTableExporter::errorString() const
{
  return m_error;
}

}
