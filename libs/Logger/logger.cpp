#include "logger.h"

#include <chrono>
#include <ctime>
#include <format>
#include <iomanip>
#include <sstream>
#include <string>

namespace qds
{

namespace
{

std::string formatTimeWithMs(int64_t micro_val) {
  using namespace std::chrono;

  // 1. Переводим микросекунды в time_point системных часов
  microseconds dur(micro_val);
  system_clock::time_point tp(duration_cast<system_clock::duration>(dur));

  // 2. Отделяем целые секунды
  auto tp_seconds = time_point_cast<seconds>(tp);

  // 3. Получаем остаток в миллисекундах (вместо микросекунд)
  auto duration_millis = duration_cast<milliseconds>(tp - tp_seconds);

  // 4. Преобразуем секунды в структуру даты (UTC)
  std::time_t tt = system_clock::to_time_t(tp_seconds);
  std::tm gmt;

#if defined(_MSC_VER)
  gmtime_s(&gmt, &tt);
#else
  gmtime_r(&tt, &gmt); // Для macOS (Xcode) и Linux
#endif

  // 5. Форматируем дату до секунд
  char buf[24];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &gmt);

  // 6. Собираем строку с 3 знаками миллисекунд
  std::ostringstream oss;
  oss << buf << '.' << std::setfill('0') << std::setw(3) << duration_millis.count();

  return oss.str();
}

} // namespace

Logger::Logger(
  const std::filesystem::path& directory,
  const IClock& clock)
  : m_directory(directory)
  , m_clock(clock)
{
  std::error_code ec;
  std::filesystem::create_directories(m_directory, ec);
}

Logger::~Logger()
{
  if (m_stream.is_open())
    m_stream.close();
}

bool Logger::debug(std::string_view message)
{
  return write(LogLevel::Debug, message);
}

bool Logger::info(std::string_view message)
{
  return write(LogLevel::Info, message);
}

bool Logger::warning(std::string_view message)
{
  return write(LogLevel::Warning, message);
}

bool Logger::error(std::string_view message)
{
  return write(LogLevel::Error, message);
}

bool Logger::write(
  LogLevel level,
  std::string_view message)
{
  const auto wallTime = m_clock.wallClockTime();
  const auto fileName = makeFileName(wallTime);

  if (!m_stream.is_open() ||
      fileName != m_currentFileName)
  {
    if (!openFile(fileName))
      return false;

    m_currentFileName = fileName;
  }

  const auto text =
    std::format(
      "{} [{}] {}\n",
      formatTimeWithMs(wallTime.unixMicroseconds),
      logLevelToString(level),
      message);

  m_stream.write(
    text.data(),
    text.size());

  return bool(m_stream);
}

bool Logger::openFile(
  const std::filesystem::path& path)
{
  if (m_stream.is_open())
    m_stream.close();

  m_stream.clear();

  m_stream.open(
    path,
    std::ios::out | std::ios::app);

  return bool(m_stream);
}

std::filesystem::path Logger::makeFileName(
  const WallClockTime& time) const
{
  const std::time_t seconds =
    time.unixMicroseconds / 1'000'000;

  std::tm tm_time{};

#if defined(_WIN32)
  gmtime_s(&tm_time, &seconds);
#else
  gmtime_r(&seconds, &tm_time);
#endif

  char buffer[11];

  std::strftime(
    buffer,
    sizeof(buffer),
    "%Y-%m-%d",
    &tm_time);

  return m_directory / (std::string(buffer) + ".log");
}

} // namespace qds