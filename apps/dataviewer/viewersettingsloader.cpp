#include "viewersettingsloader.h"
#include "datatypes.h"

#include <QSettings>
#include <QStringList>

namespace qds
{

namespace
{

bool validFrequency(uint32_t frequency)
{
  switch (frequency)
  {
  case 1:
  case 10:
  case 100:
  case 1000:
    return true;

  default:
    return false;
  }
}

}

std::vector<ViewerTableSettings>
ViewerSettingsLoader::loadTables(
  const QString& fileName)
{
  QSettings settings(
    fileName,
    QSettings::IniFormat);

  const int count =
    settings.beginReadArray("ViewerTables");

  std::vector<ViewerTableSettings> result;

  result.reserve(count);

  for (int i = 0; i < count; ++i)
  {
    settings.setArrayIndex(i);

    ViewerTableSettings table;

    const uint32_t frequency =
      settings.value("Frequency", 100).toUInt();

    table.frequency =
      validFrequency(frequency)
        ? frequency
        : 100;

    const QString text =
      settings.value("Channels").toString();

    const QStringList values =
      text.split(',', Qt::SkipEmptyParts);

    table.channels.reserve(values.size());

    for (const QString& value : values)
    {
      bool ok = false;

      const uint32_t id =
        value.trimmed().toUInt(&ok);

      if (ok && id != 0)
        table.channels.push_back(SignalId{id});
    }

    result.push_back(std::move(table));
  }

  settings.endArray();

  return result;
}

bool ViewerSettingsLoader::saveTables(
  const QString& fileName,
  const std::vector<ViewerTableSettings>& tables)
{
  if (fileName.isEmpty())
    return false;

  QSettings settings(
    fileName,
    QSettings::IniFormat);

  // Удаляем прежний список, чтобы не оставались
  // настройки удалённых вкладок.
  settings.remove("ViewerTables");

  settings.beginWriteArray(
    "ViewerTables",
    static_cast<int>(tables.size()));

  for (std::size_t i = 0; i < tables.size(); ++i)
  {
    settings.setArrayIndex(
      static_cast<int>(i));

    const auto& table = tables[i];

    settings.setValue(
      "Frequency",
      validFrequency(table.frequency)
        ? table.frequency
        : 100u);

    QStringList channels;

    for (SignalId id : table.channels)
    {
      channels.append(
        QString::number(id.value));
    }

    settings.setValue(
      "Channels",
      channels.join(','));
  }

  settings.endArray();

  settings.sync();

  return settings.status() == QSettings::NoError;
}


}