#pragma once

#include "ilogger.h"
#include "loglevel.h"

#include <filesystem>
#include <fstream>
#include <string_view>

namespace qds
{

class Logger : public ILogger
{
public:
  explicit Logger(
    const std::filesystem::path& path);

  ~Logger();

  bool debug(std::string_view message) override;
  bool info(std::string_view message) override;
  bool warning(std::string_view message) override;
  bool error(std::string_view message) override;

private:
  bool write(
    LogLevel level,
    std::string_view message);

  std::fstream m_stream;
};

}