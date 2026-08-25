#include "archivedescriptionwriter.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace qds
{

namespace
{

QString signalKindToString(
  SignalKind kind)
{
  switch (kind)
  {
  case SignalKind::Raw:
    return "Raw";

  case SignalKind::Calculated:
    return "Calculated";

  default:
    return "Unknown";
  }
}

QJsonObject signalToJson(
  const ArchiveSignal& signal)
{
  QJsonObject object;

  object["index"] =
    static_cast<int>(
      signal.index);

  object["id"] =
    static_cast<int>(
      signal.id.value);

  object["name"] =
    QString::fromStdString(
      signal.name);

  object["kind"] =
    signalKindToString(
      signal.kind);

  if (signal.module.has_value())
  {
    object["module"] =
      static_cast<int>(
        signal.module->value);
  }

  if (signal.channel.has_value())
  {
    object["channel"] =
      static_cast<int>(
        signal.channel->value);
  }

  return object;
}

QJsonObject fileToJson(
  const ArchiveFileDescription& file)
{
  QJsonObject object;

  object["name"] =
    QString::fromStdString(
      file.name);

  object["frequency"] =
    static_cast<int>(
      file.frequency);

  object["dataType"] =
    QString::fromStdString(
      file.dataType);

  QJsonArray signalIds;

  for (const auto& signal :
       file.signalIds)
  {
    signalIds.append(
      signalToJson(signal));
  }

  object["signals"] =
    signalIds;

  return object;
}

} // namespace

bool ArchiveDescriptionWriter::write(
  const std::filesystem::path& path,
  const ArchiveDescription& description) const
{
  QJsonObject root;

  root["version"] =
    static_cast<int>(
      description.version);

  QJsonArray files;

  for (const auto& file :
       description.files)
  {
    files.append(
      fileToJson(file));
  }

  root["files"] =
    files;

  const QJsonDocument document(root);

  QFile output(
    QString::fromStdString(
      path.string()));

  if (!output.open(
        QIODevice::WriteOnly |
        QIODevice::Truncate))
  {
    return false;
  }

  const QByteArray data =
    document.toJson(
      QJsonDocument::Indented);

  return output.write(data) ==
         data.size();
}

} // namespace qds