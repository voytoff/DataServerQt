#include "monitorsettingsloader.h"

#include <QSettings>
#include <QStringList>

namespace qds
{

std::vector<SignalId> MonitorSettingsLoader::loadSignals(
  const QString& fileName,
  ConfigurationId configurationId)
{
  QSettings settings(
    fileName,
    QSettings::IniFormat);

  const QString key =
    QString("MonitorSignals/%1")
      .arg(configurationId.value);

  const QString text =
    settings.value(key).toString();

  std::vector<SignalId> result;

  if (text.isEmpty())
    return result;

  const QStringList values =
    text.split(',', Qt::SkipEmptyParts);

  result.reserve(values.size());

  for (const QString& value : values)
  {
    bool ok = false;

    const uint32_t id =
      value.trimmed().toUInt(&ok);

    if (!ok || id == 0)
      continue;

    result.push_back(SignalId{id});
  }

  return result;
}

bool MonitorSettingsLoader::saveSignals(
  const QString& fileName,
  ConfigurationId configurationId,
  const std::vector<SignalId>& signalIds)
{
  if (configurationId.value == 0)
    return false;

  QSettings settings(
    fileName,
    QSettings::IniFormat);

  const QString key =
    QString("MonitorSignals/%1")
      .arg(configurationId.value);

  QStringList values;

  for (SignalId id : signalIds)
  {
    values.append(
      QString::number(id.value));
  }

  settings.setValue(
    key,
    values.join(','));

  settings.sync();

  return settings.status() == QSettings::NoError;
}

}