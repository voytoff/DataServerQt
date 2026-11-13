
#include "archivedescriptionreader.h"
#include "archiveformat.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace qds
{

namespace
{

SignalKind signalKindFromString(
  const std::string& value)
{
  if (value == "Raw")
    return SignalKind::Raw;

  if (value == "Calculated")
    return SignalKind::Calculated;

  return static_cast<SignalKind>(-1);
}

bool signalFromJson(
  const QJsonObject& object,
  ArchiveSignal& signal)
{
  signal.index =
    static_cast<uint32_t>(
      object["index"].toInt());

  signal.id =
    SignalId{
      static_cast<uint32_t>(
        object["id"].toInt())
    };

  signal.name =
    object["name"].toString().toStdString();

  signal.unit =
    object.value("unit").toString().toStdString();

  signal.kind =
    signalKindFromString(
      object["kind"].toString().toStdString());

  if (signal.kind == SignalKind::Raw)
  {
    if (!object.contains("channel") ||
        !object.contains("module"))
    {
      return false;
    }

    signal.channel =
      ChannelId{
        static_cast<uint32_t>(
          object["channel"].toInt())
      };

    signal.module =
      ModuleId{
        static_cast<uint32_t>(
          object["module"].toInt())
      };
  }
  else if (signal.kind == SignalKind::Calculated)
  {
    if (object.contains("channel") ||
        object.contains("module"))
    {
      return false;
    }
  }
  else
  {
    return false;
  }

  return true;
}

bool fileFromJson(
  const QJsonObject& object,
  ArchiveFileDescription& file)
{
  file.frequency =
    static_cast<uint32_t>(
      object["frequency"].toInt());

  if (!isValidArchiveFrequency(file.frequency))
    return false;

  file.dataType =
    object["dataType"].toString().toStdString();

  if (file.dataType != "float")
    return false;

  file.name =
    object["name"].toString().toStdString();

  if (!object["signals"].isArray())
    return false;

  const QJsonArray signalIds =
    object["signals"].toArray();

  for (const auto& item : signalIds)
  {
    if (!item.isObject())
      return false;

    ArchiveSignal signal;

    if (!signalFromJson(
          item.toObject(),
          signal))
    {
      return false;
    }

    file.signalIds.push_back(
      std::move(signal));
  }

  return !file.signalIds.empty();
}

} // namespace

bool ArchiveDescriptionReader::read(
  const std::filesystem::path& path,
  ArchiveDescription& description) const
{
  QFile input(
    QString::fromStdString(
      path.string()));

  if (!input.open(QIODevice::ReadOnly))
    return false;

  const QByteArray data = input.readAll();

  if (data.isEmpty())
    return false;

  QJsonParseError error;

  const QJsonDocument document =
    QJsonDocument::fromJson(
      data,
      &error);

  if (error.error != QJsonParseError::NoError ||
      !document.isObject())
  {
    return false;
  }

  const QJsonObject root =
    document.object();

  ArchiveDescription tmp;

  tmp.version =
    root["version"].toInt();

  if (tmp.version != ArchiveDescriptionVersion)
    return false;

  if (!root["files"].isArray())
    return false;

  const QJsonArray files =
    root["files"].toArray();

  if (files.isEmpty())
    return false;

  for (const auto& item : files)
  {
    if (!item.isObject())
      return false;

    ArchiveFileDescription file;

    if (!fileFromJson(
          item.toObject(),
          file))
    {
      return false;
    }

    tmp.files.push_back(
      std::move(file));
  }

  description = std::move(tmp);

  return true;
}

} // namespace qds
