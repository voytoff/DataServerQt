#include "guilogger.h"

namespace qds
{

GuiLogger::GuiLogger(
  ILogger &fileLogger,
  QObject *parent)
  : QObject(parent)
  , m_fileLogger(fileLogger)
{
}

bool GuiLogger::debug(
  std::string_view message)
{
  write(LogLevel::Debug, message);
  return m_fileLogger.debug(message);
}

bool GuiLogger::info(
  std::string_view message)
{
  write(LogLevel::Info, message);
  return m_fileLogger.info(message);
}

bool GuiLogger::warning(
  std::string_view message)
{
  write(LogLevel::Warning, message);
  return m_fileLogger.warning(message);
}

bool GuiLogger::error(
  std::string_view message)
{
  write(LogLevel::Error, message);
  return m_fileLogger.error(message);
}

void GuiLogger::write(LogLevel level, std::string_view message)
{
  emit messageReceived(
    level,
    QString::fromUtf8(
      message.data(),
      message.size()));
}

}
