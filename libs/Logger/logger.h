#pragma once

#include "iclock.h"
#include "loglevel.h"
#include "ilogger.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string_view>

namespace qds
{

class Logger : public ILogger
{
public:
  explicit Logger(
    const std::filesystem::path& directory,
    const IClock& clock);

  ~Logger();

  bool debug(std::string_view message) override;
  bool info(std::string_view message) override;
  bool warning(std::string_view message) override;
  bool error(std::string_view message) override;

private:
  bool write(
    LogLevel level,
    std::string_view message);

  bool openFile(
    const std::filesystem::path& path);

  std::filesystem::path makeFileName(
    const WallClockTime& time) const;

private:
  std::filesystem::path m_directory;
  const IClock& m_clock;

  std::filesystem::path m_currentFileName;
  std::ofstream m_stream;

  std::mutex m_mutex;
};

}