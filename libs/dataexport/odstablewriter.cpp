#include "odstablewriter.h"

#include <QFile>
#include <QTemporaryDir>
#include <QXmlStreamWriter>

namespace qds
{

namespace
{

constexpr uint32_t MaxWorksheetRows = 1'048'576;
constexpr uint32_t MaxWorksheetColumns = 16'384;

}

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
        headers.size() > MaxWorksheetColumns)
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

    if (!writeXmlHeader() || !beginSheet())
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
      QStringLiteral("office:document-content"));

    xml.writeNamespace(
      QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:office:1.0"),
      QStringLiteral("office"));

    xml.writeNamespace(
      QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:table:1.0"),
      QStringLiteral("table"));

    xml.writeNamespace(
      QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:text:1.0"),
      QStringLiteral("text"));

    xml.writeAttribute(
      QStringLiteral("office:version"),
      QStringLiteral("1.2"));

    xml.writeStartElement(
      QStringLiteral("office:body"));

    xml.writeStartElement(
      QStringLiteral("office:spreadsheet"));

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

}