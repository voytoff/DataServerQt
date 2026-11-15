
#include "xlsxtablewriter.h"

#include <xlsxwriter.h>

#include <QDateTime>

#include <filesystem>
#include <string>
#include <system_error>
#include <utility>

namespace qds
{

namespace
{

constexpr uint32_t MaxWorksheetRows = 1'048'576;
constexpr uint32_t MaxWorksheetColumns = 16'384;

QString xlsxError(lxw_error error)
{
  return QString::fromUtf8(lxw_strerror(error));
}

}

class XlsxTableWriter::Impl
{
public:
  explicit Impl(std::filesystem::path path)
    : outputPath(std::move(path))
  {
  }

  ~Impl()
  {
    if (workbook)
      abort();
  }

  bool begin(const std::vector<QString>& headers)
  {
    error.clear();

    if (workbook)
    {
      error = QStringLiteral(
        "Экспорт уже выполняется");
      return false;
    }

    if (headers.size() < 2 ||
        headers.size() > MaxWorksheetColumns)
    {
      error = QStringLiteral(
        "Недопустимое количество столбцов");
      return false;
    }

    this->headers = headers;
    sheetIndex = 0;
    currentRow = 0;

    const auto path = outputPath.u8string();

    lxw_workbook_options options = {};
    options.constant_memory = LXW_TRUE;

    workbook = workbook_new_opt(
      reinterpret_cast<const char*>(path.c_str()),
      &options);

    if (!workbook)
    {
      error = QStringLiteral(
        "Не удалось создать XLSX");
      return false;
    }

    dateFormat = workbook_add_format(workbook);

    if (!dateFormat)
    {
      error = QStringLiteral(
        "Не удалось создать формат даты");
      return false;
    }

    format_set_num_format(
      dateFormat,
      "dd.mm.yyyy hh:mm:ss.000");

    return createWorksheet();
  }

  bool createWorksheet()
  {
    currentWorksheet =
      workbook_add_worksheet(workbook, nullptr);

    if (!currentWorksheet)
    {
      error = QStringLiteral(
        "Не удалось создать лист XLSX");
      return false;
    }

    ++sheetIndex;
    currentRow = 0;

    for (std::size_t column = 0;
         column < headers.size();
         ++column)
    {
      const QByteArray header =
        headers[column].toUtf8();

      const lxw_error result =
        worksheet_write_string(
          currentWorksheet,
          0,
          static_cast<lxw_col_t>(column),
          header.constData(),
          nullptr);

      if (result != LXW_NO_ERROR)
      {
        error = xlsxError(result);
        return false;
      }
    }

    currentRow = 1;

    return true;
  }

  bool writeRow(
    double seconds,
    int64_t wallMilliseconds,
    std::span<const std::optional<double>> values)
  {
    if (!workbook || !currentWorksheet)
    {
      error = QStringLiteral(
        "XLSX не инициализирован");
      return false;
    }

    if (values.size() + 2 != headers.size())
    {
      error = QStringLiteral(
        "Количество значений не соответствует заголовкам");
      return false;
    }

    if (currentRow >= MaxWorksheetRows)
    {
      if (!createWorksheet())
        return false;
    }

    lxw_error result = worksheet_write_number(
      currentWorksheet,
      currentRow,
      0,
      seconds,
      nullptr);

    if (result != LXW_NO_ERROR)
    {
      error = xlsxError(result);
      return false;
    }

    const QDateTime dateTime =
      QDateTime::fromMSecsSinceEpoch(
        wallMilliseconds);

    const QDate date = dateTime.date();
    const QTime time = dateTime.time();

    lxw_datetime excelDateTime = {};

    excelDateTime.year = date.year();
    excelDateTime.month = date.month();
    excelDateTime.day = date.day();
    excelDateTime.hour = time.hour();
    excelDateTime.min = time.minute();

    excelDateTime.sec =
      time.second() + time.msec() / 1000.0;

    result = worksheet_write_datetime(
      currentWorksheet,
      currentRow,
      1,
      &excelDateTime,
      dateFormat);

    if (result != LXW_NO_ERROR)
    {
      error = xlsxError(result);
      return false;
    }

    for (std::size_t column = 0;
         column < values.size();
         ++column)
    {
      if (!values[column])
        continue;

      result = worksheet_write_number(
        currentWorksheet,
        currentRow,
        static_cast<lxw_col_t>(column + 2),
        *values[column],
        nullptr);

      if (result != LXW_NO_ERROR)
      {
        error = xlsxError(result);
        return false;
      }
    }

    ++currentRow;

    return true;
  }

  bool finish()
  {
    if (!workbook)
    {
      error = QStringLiteral(
        "XLSX не инициализирован");
      return false;
    }

    lxw_workbook* closingWorkbook =
      std::exchange(workbook, nullptr);

    currentWorksheet = nullptr;
    dateFormat = nullptr;

    const lxw_error result =
      workbook_close(closingWorkbook);

    if (result != LXW_NO_ERROR)
    {
      error = xlsxError(result);
      return false;
    }

    return true;
  }

  void abort() noexcept
  {
    if (workbook)
    {
      lxw_workbook* closingWorkbook =
        std::exchange(workbook, nullptr);

      currentWorksheet = nullptr;
      dateFormat = nullptr;

      workbook_close(closingWorkbook);
    }

    std::error_code ec;
    std::filesystem::remove(outputPath, ec);
  }

  QString errorString() const
  {
    return error;
  }

private:
  std::filesystem::path outputPath;

  lxw_workbook* workbook = nullptr;
  lxw_worksheet* currentWorksheet = nullptr;
  lxw_format* dateFormat = nullptr;

  std::vector<QString> headers;

  uint32_t sheetIndex = 0;
  uint32_t currentRow = 0;

  QString error;
};

XlsxTableWriter::XlsxTableWriter(
  std::filesystem::path outputPath)
  : m_impl(std::make_unique<Impl>(
      std::move(outputPath)))
{
}

XlsxTableWriter::~XlsxTableWriter() = default;

bool XlsxTableWriter::begin(
  const std::vector<QString>& headers)
{
  return m_impl->begin(headers);
}

bool XlsxTableWriter::writeRow(
  double seconds,
  int64_t wallMilliseconds,
  std::span<const std::optional<double>> values)
{
  return m_impl->writeRow(
    seconds,
    wallMilliseconds,
    values);
}

bool XlsxTableWriter::finish()
{
  return m_impl->finish();
}

void XlsxTableWriter::abort() noexcept
{
  m_impl->abort();
}

QString XlsxTableWriter::errorString() const
{
  return m_impl->errorString();
}

}
