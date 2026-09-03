#pragma once

#include <string>

namespace qds
{

enum class LogLevel
{
  Debug,
  Info,
  Warning,
  Error
};

inline std::string logLevelToString(LogLevel value)
{
  switch (value)
  {
  case LogLevel::Debug:
    return "DEBUG";

  case LogLevel::Info:
    return "INFO";

  case LogLevel::Warning:
    return "WARN";

  case LogLevel::Error:
    return "ERROR";
  }

  return {};
}

}