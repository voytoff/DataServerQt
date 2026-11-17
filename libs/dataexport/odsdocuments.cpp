#include "odsdocuments.h"
#include "QXmlStreamWriter"
#include "QDateTime"

namespace qds
{

QByteArray buildMimeType()
{
  return QByteArrayLiteral(
    "application/vnd.oasis.opendocument.spreadsheet");
}

QByteArray buildMetaXml()
{
  QByteArray data;
  QXmlStreamWriter xml(&data);
  xml.setAutoFormatting(true);
  xml.writeStartDocument();

  xml.writeStartElement(
    ods::nsOffice,
    QStringLiteral("document-meta"));

  xml.writeNamespace(
    ods::nsOffice,
    QStringLiteral("office"));

  xml.writeNamespace(
    ods::nsMeta,
    QStringLiteral("meta"));

  xml.writeAttribute(ods::nsOffice, QStringLiteral("version"), QStringLiteral("1.2"));

  xml.writeStartElement(ods::nsOffice, QStringLiteral("meta"));
  xml.writeTextElement(ods::nsMeta, QStringLiteral("generator"), QStringLiteral("Qt-dataexport/1.0"));
  xml.writeTextElement(
    ods::nsMeta,
    QStringLiteral("creation-date"),
    QDateTime::currentDateTimeUtc()
      .toString(Qt::ISODateWithMs));
  xml.writeEndElement(); // office:meta

  xml.writeEndElement(); // office:document-meta
  xml.writeEndDocument();

  return xml.hasError() ? QByteArray{} : data;
}

QByteArray buildManifestXml()
{
  QByteArray data;
  QXmlStreamWriter xml(&data);
  xml.setAutoFormatting(true);
  xml.writeStartDocument();

  xml.writeStartElement(
    ods::nsManifest,
    QStringLiteral("manifest"));

  xml.writeNamespace(ods::nsOffice, QStringLiteral("office"));

  xml.writeNamespace(
    ods::nsManifest,
    QStringLiteral("manifest"));

  xml.writeAttribute(ods::nsManifest, QStringLiteral("version"), QStringLiteral("1.2"));

  auto addEntry = [&](const QString &path, const QString &mediaType) {
    xml.writeEmptyElement(ods::nsManifest, QStringLiteral("file-entry"));
    xml.writeAttribute(ods::nsManifest, QStringLiteral("full-path"), path);
    xml.writeAttribute(ods::nsManifest, QStringLiteral("media-type"), mediaType);
  };

  // Корневая запись архива обязательна
  addEntry(QStringLiteral("/"), QStringLiteral("application/vnd.oasis.opendocument.spreadsheet"));
  addEntry(QStringLiteral("content.xml"), QStringLiteral("text/xml"));
  addEntry(QStringLiteral("styles.xml"), QStringLiteral("text/xml"));
  addEntry(QStringLiteral("meta.xml"), QStringLiteral("text/xml"));

  xml.writeEndElement(); // manifest:manifest
  xml.writeEndDocument();

  return xml.hasError() ? QByteArray{} : data;
}

QByteArray buildStylesXml()
{
  QByteArray data;
  QXmlStreamWriter xml(&data);
  xml.setAutoFormatting(true);
  xml.writeStartDocument();

  xml.writeStartElement(
    ods::nsOffice,
    QStringLiteral("document-styles"));

  xml.writeNamespace(ods::nsOffice, QStringLiteral("office"));
  xml.writeNamespace(ods::nsStyle, QStringLiteral("style"));
  xml.writeNamespace(ods::nsFo, QStringLiteral("fo"));

  xml.writeAttribute(
    ods::nsOffice,
    QStringLiteral("version"),
    QStringLiteral("1.2"));

  // Базовые настройки страницы по умолчанию
  xml.writeStartElement(ods::nsOffice, QStringLiteral("automatic-styles"));
  xml.writeStartElement(ods::nsStyle, QStringLiteral("page-layout"));
  xml.writeAttribute(ods::nsStyle, QStringLiteral("name"), QStringLiteral("pm1"));
  xml.writeStartElement(ods::nsStyle, QStringLiteral("page-layout-properties"));
  xml.writeAttribute(ods::nsFo, QStringLiteral("margin-top"), QStringLiteral("2cm"));
  xml.writeAttribute(ods::nsFo, QStringLiteral("margin-bottom"), QStringLiteral("2cm"));
  xml.writeAttribute(ods::nsFo, QStringLiteral("margin-left"), QStringLiteral("2cm"));
  xml.writeAttribute(ods::nsFo, QStringLiteral("margin-right"), QStringLiteral("2cm"));
  xml.writeEndElement(); // style:page-layout-properties
  xml.writeEndElement(); // style:page-layout
  xml.writeEndElement(); // office:automatic-styles

  xml.writeStartElement(ods::nsOffice, QStringLiteral("master-styles"));
  xml.writeStartElement(ods::nsStyle, QStringLiteral("master-page"));
  xml.writeAttribute(ods::nsStyle, QStringLiteral("name"), QStringLiteral("Default"));
  xml.writeAttribute(ods::nsStyle, QStringLiteral("page-layout-name"), QStringLiteral("pm1"));
  xml.writeEndElement(); // style:master-page
  xml.writeEndElement(); // office:master-styles

  xml.writeEndElement(); // office:document-styles
  xml.writeEndDocument();

  return xml.hasError() ? QByteArray{} : data;
}

}