#include "odstablewriter.h"
#include "odsdocuments.h"
#include "odspackage.h"

#include <QFile>
#include <QTemporaryDir>
#include <QXmlStreamWriter>

namespace qds
{

class OdsTableWriter::Impl
{
public:
  explicit Impl(std::filesystem::path path)
    : outputPath(std::move(path))
    , xml(&contentFile)
  {
    xml.setAutoFormatting(false);
  }

  bool begin(const std::vector<QString>& headers)
  {
    error.clear();

    if (headers.size() < 2 ||
        headers.size() > ods::MaxWorksheetColumns)
    {
      error = QStringLiteral(
        "Недопустимое количество столбцов");
      return false;
    }

    if (!tempDir.isValid())
    {
      error = QStringLiteral(
        "Ошибка создания временного каталога: %1")
        .arg(tempDir.errorString());

      return false;
    }

    contentFile.setFileName(
      tempDir.filePath(QStringLiteral("content.xml")));

    if (!contentFile.open(QIODevice::WriteOnly))
    {
      error = QStringLiteral(
        "Ошибка создания content.xml: %1")
        .arg(contentFile.errorString());

      return false;
    }

    this->headers = headers;
    sheetIndex = 0;
    currentRow = 0;

    if (!writeXmlHeader() ||
        !writeStyles() ||
        !beginSpreadsheet() ||
        !beginSheet())
    {
      error = QStringLiteral(
        "Ошибка записи заголовка ODS");
      return false;
    }

    return true;
  }

  bool writeXmlHeader()
  {
    xml.writeStartDocument();

    xml.writeStartElement(
      ods::nsOffice,
      QStringLiteral("document-content"));

    xml.writeNamespace(ods::nsOffice, QStringLiteral("office"));
    xml.writeNamespace(ods::nsTable, QStringLiteral("table"));
    xml.writeNamespace(ods::nsText, QStringLiteral("text"));
    xml.writeNamespace(ods::nsStyle, QStringLiteral("style"));
    xml.writeNamespace(ods::nsNumber, QStringLiteral("number"));

    xml.writeAttribute(
      ods::nsOffice,
      QStringLiteral("version"),
      QStringLiteral("1.2"));

    return !xml.hasError();
  }

  bool writeStyles()
  {
    xml.writeStartElement(ods::nsOffice, QStringLiteral("automatic-styles"));

    // 1. Описываем формат даты: dd.MM.yyyy HH:mm:ss.000
    xml.writeStartElement(ods::nsNumber, QStringLiteral("date-style"));
    xml.writeAttribute(ods::nsStyle, QStringLiteral("name"), QStringLiteral("N100"));

    // dd
    xml.writeStartElement(ods::nsNumber, QStringLiteral("day"));
    xml.writeAttribute(ods::nsNumber, QStringLiteral("style"), QStringLiteral("long"));
    xml.writeEndElement();

    xml.writeTextElement(ods::nsNumber, QStringLiteral("text"), QStringLiteral("."));

    // MM
    xml.writeStartElement(ods::nsNumber, QStringLiteral("month"));
    xml.writeAttribute(ods::nsNumber, QStringLiteral("style"), QStringLiteral("long"));
    xml.writeEndElement();

    xml.writeTextElement(ods::nsNumber, QStringLiteral("text"), QStringLiteral("."));

    // yyyy
    xml.writeStartElement(ods::nsNumber, QStringLiteral("year"));
    xml.writeAttribute(ods::nsNumber, QStringLiteral("style"), QStringLiteral("long"));
    xml.writeEndElement();

    xml.writeTextElement(ods::nsNumber, QStringLiteral("text"), QStringLiteral(" "));

    // HH
    xml.writeStartElement(ods::nsNumber, QStringLiteral("hours"));
    xml.writeAttribute(ods::nsNumber, QStringLiteral("style"), QStringLiteral("long"));
    xml.writeEndElement();

    xml.writeTextElement(ods::nsNumber, QStringLiteral("text"), QStringLiteral(":"));

    // mm
    xml.writeStartElement(ods::nsNumber, QStringLiteral("minutes"));
    xml.writeAttribute(ods::nsNumber, QStringLiteral("style"), QStringLiteral("long"));
    xml.writeEndElement();

    xml.writeTextElement(ods::nsNumber, QStringLiteral("text"), QStringLiteral(":"));

    // ss.000 (секунды с 3 знаками после запятой)
    xml.writeStartElement(ods::nsNumber, QStringLiteral("seconds"));
    xml.writeAttribute(ods::nsNumber, QStringLiteral("style"), QStringLiteral("long"));
    xml.writeAttribute(ods::nsNumber, QStringLiteral("decimal-places"), QStringLiteral("3"));
    xml.writeEndElement();

    xml.writeEndElement(); // number:date-style

    // 2. Создаем стиль ячейки (ce1) и привязываем к нему формат N100
    xml.writeStartElement(ods::nsStyle, QStringLiteral("style"));
    xml.writeAttribute(ods::nsStyle, QStringLiteral("name"), QStringLiteral("ce1"));
    xml.writeAttribute(ods::nsStyle, QStringLiteral("family"), QStringLiteral("table-cell"));
    xml.writeAttribute(ods::nsStyle, QStringLiteral("data-style-name"), QStringLiteral("N100"));
    xml.writeEndElement(); // style:style

    xml.writeEndElement(); // office:automatic-styles

    if (xml.hasError())
    {
      error = xml.errorString();
      return false;
    }

    return true;
  }

  bool beginSpreadsheet()
  {
    xml.writeStartElement(
      ods::nsOffice,
      QStringLiteral("body"));

    xml.writeStartElement(
      ods::nsOffice,
      QStringLiteral("spreadsheet"));

    return !xml.hasError();
  }

  bool beginSheet()
  {
    ++sheetIndex;
    currentRow = 0;

    xml.writeStartElement(
      QStringLiteral("table:table"));

    xml.writeAttribute(
      QStringLiteral("table:name"),
      QStringLiteral("Sheet%1").arg(sheetIndex));

    xml.writeStartElement(
      QStringLiteral("table:table-row"));

    for (const QString& header : headers)
    {
      xml.writeStartElement(
        QStringLiteral("table:table-cell"));

      xml.writeAttribute(
        QStringLiteral("office:value-type"),
        QStringLiteral("string"));

      xml.writeTextElement(
        QStringLiteral("text:p"),
        header);

      xml.writeEndElement(); // table:table-cell
    }

    xml.writeEndElement(); // table:table-row

    currentRow = 1;

    return !xml.hasError();
  }

  bool writeRow(
    double seconds,
    int64_t wallMilliseconds,
    std::span<const std::optional<double>> values)
  {
    if (xml.hasError())
    {
      error = xml.errorString();
      return false;
    }

    if (values.size() + 2 != headers.size())
    {
      error = QStringLiteral(
        "Количество значений не соответствует заголовкам");
      return false;
    }

    if (currentRow >= ods::MaxWorksheetRows)
    {
      if (!endSheet() || !beginSheet())
        return false;
    }

    xml.writeStartElement(
      ods::nsTable,
      QStringLiteral("table-row"));

    // Секунды
    xml.writeStartElement(
      ods::nsTable,
      QStringLiteral("table-cell"));

    xml.writeAttribute(
      ods::nsOffice,
      QStringLiteral("value-type"),
      QStringLiteral("float"));

    xml.writeAttribute(
      ods::nsOffice,
      QStringLiteral("value"),
      QString::number(seconds, 'g', 17));

    xml.writeEndElement();

    // Время
    const QDateTime dateTime =
      QDateTime::fromMSecsSinceEpoch(wallMilliseconds);

    xml.writeStartElement(
      ods::nsTable,
      QStringLiteral("table-cell"));

    // Привязываем стиль ячейки ce1
    xml.writeAttribute(ods::nsTable, QStringLiteral("style-name"), QStringLiteral("ce1"));
    xml.writeAttribute(ods::nsOffice, QStringLiteral("value-type"), QStringLiteral("date"));
    // ISODateWithMs сформирует строку вида "2026-11-17T07:11:05.123"
    xml.writeAttribute(
      ods::nsOffice,
      QStringLiteral("date-value"),
      dateTime.toString(QStringLiteral("yyyy-MM-ddTHH:mm:ss.zzz")));
    // Отображаемый текст
    xml.writeStartElement(ods::nsText, QStringLiteral("p"));
    xml.writeCharacters(
      dateTime.toString(QStringLiteral("dd.MM.yyyy HH:mm:ss.zzz")));
    xml.writeEndElement(); // text:p

    xml.writeEndElement(); // table:table-cell

    // Значения каналов
    for (const auto& value : values)
    {
      if (!value)
      {
        xml.writeEmptyElement(
          ods::nsTable,
          QStringLiteral("table-cell"));

        continue;
      }

      xml.writeStartElement(
        ods::nsTable,
        QStringLiteral("table-cell"));

      xml.writeAttribute(
        ods::nsOffice,
        QStringLiteral("value-type"),
        QStringLiteral("float"));

      xml.writeAttribute(
        ods::nsOffice,
        QStringLiteral("value"),
        QString::number(*value, 'g', 17));

      xml.writeEndElement();
    }

    xml.writeEndElement(); // table:table-row

    if (xml.hasError())
    {
      error = xml.errorString();
      return false;
    }

    ++currentRow;
    return true;
  }

  bool finish()
  {
    if (!endDocument())
      return false;

    if (!contentFile.flush())
    {
      error = contentFile.errorString();
      return false;
    }

    contentFile.close();

    if (contentFile.error() != QFileDevice::NoError)
    {
      error = contentFile.errorString();
      return false;
    }

    OdsPackage package(outputPath);

    if (!package.begin() ||
        !package.addData(
          QStringLiteral("mimetype"),
          buildMimeType(),
          false) ||
        !package.addFile(
          QStringLiteral("content.xml"),
          contentFile.fileName()) ||
        !package.addData(
          QStringLiteral("styles.xml"),
          buildStylesXml()) ||
        !package.addData(
          QStringLiteral("meta.xml"),
          buildMetaXml()) ||
        !package.addData(
          QStringLiteral("META-INF/manifest.xml"),
          buildManifestXml()) ||
        !package.finish())
    {
      error = package.errorString();
      return false;
    }

    return true;
  }

  void abort() noexcept
  {
    if (contentFile.isOpen())
      contentFile.close();

    std::error_code ec;
    std::filesystem::remove(outputPath, ec);
  }

  QString errorString() const
  {
    return error;
  }

  bool endSheet()
  {
    xml.writeEndElement(); // table:table

    if (xml.hasError())
    {
      error = xml.errorString();
      return false;
    }

    return true;
  }

  bool endDocument()
  {
    if (!endSheet())
      return false;

    xml.writeEndElement(); // office:spreadsheet
    xml.writeEndElement(); // office:body
    xml.writeEndElement(); // office:document-content

    xml.writeEndDocument();

    if (xml.hasError())
    {
      error = xml.errorString();
      return false;
    }

    return true;
  }

private:
  std::filesystem::path outputPath;

  QTemporaryDir tempDir;
  QFile contentFile;
  QXmlStreamWriter xml;

  std::vector<QString> headers;

  uint32_t sheetIndex = 0;
  uint32_t currentRow = 0;

  QString error;
};

OdsTableWriter::OdsTableWriter(
  std::filesystem::path outputPath)
  : m_impl(std::make_unique<Impl>(
      std::move(outputPath)))
{
}

OdsTableWriter::~OdsTableWriter() = default;

bool OdsTableWriter::begin(
  const std::vector<QString>& headers)
{
  return m_impl->begin(headers);
}

bool OdsTableWriter::writeRow(
  double seconds,
  int64_t wallMilliseconds,
  std::span<const std::optional<double> > values)
{
  return m_impl->writeRow(
    seconds,
    wallMilliseconds,
    values);
}

bool OdsTableWriter::finish()
{
  return m_impl->finish();
}

void OdsTableWriter::abort() noexcept
{
  m_impl->abort();
}

QString OdsTableWriter::errorString() const
{
  return m_impl->errorString();
}

}