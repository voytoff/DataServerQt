#include "logger.h"

#include <chrono>
#include <format>
#include <iomanip>
#include <sstream>
#include <string>

namespace qds
{

namespace
{

std::string formatTimeWithMs(
  std::chrono::system_clock::time_point tp)
{
  const auto time_c =
    std::chrono::system_clock::to_time_t(tp);

  const std::tm tm_local =
    *std::localtime(&time_c);

  const auto duration =
    tp.time_since_epoch();

  const auto seconds =
    std::chrono::duration_cast<
      std::chrono::seconds>(
      duration);

  const auto milliseconds =
    std::chrono::duration_cast<
      std::chrono::milliseconds>(
      duration - seconds);

  std::stringstream ss;

  ss << std::put_time(
    &tm_local,
    "%Y-%m-%d %H:%M:%S")
     << '.'
     << std::setfill('0')
     << std::setw(3)
     << milliseconds.count();

  return ss.str();
}

} // namespace

Logger::Logger(
  const std::filesystem::path& path)
{
  m_stream.open(
    path,
    std::ios::out | std::ios::trunc);
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
  if (!m_stream.is_open())
    return false;

  const auto now =
    std::chrono::system_clock::now();

  const auto text =
    std::format(
      "{} [{}] {}\n",
      formatTimeWithMs(now),
      logLevelToString(level),
      message);

  m_stream.write(
    text.data(),
    text.size());

  if (!m_stream)
    return false;

  return true;
}

} // namespace qds