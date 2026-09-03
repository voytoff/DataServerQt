#include "tst_logger.h"
#include "systemclock.h"
#include "testsrv.h"
#include "logger.h"
#include <fstream>

tst_logger::tst_logger() { }
tst_logger::~tst_logger() = default;

static std::filesystem::path findLogFile(std::filesystem::path directory)
{
  std::filesystem::path logFile;

  for (const auto& entry :
       std::filesystem::directory_iterator(directory))
  {
    if (entry.is_regular_file() &&
        entry.path().string().contains("2026") &&
        entry.path().extension() == ".log")
    {
      logFile = entry.path();
      break;
    }
  }

  return logFile;
}


void tst_logger::test_logger_base()
{
  const auto path = getCurrentFolder();
  SystemClock clock;

  {
    Logger logger(path, clock);

    QVERIFY(logger.info("DataServer started"));
  }

  const auto &logFile = findLogFile(path);

  std::ifstream stream(logFile);

  QVERIFY(stream.is_open());

  std::string line;
  QVERIFY(std::getline(stream, line));

  QVERIFY(line.find("[INFO]") != std::string::npos);
  QVERIFY(line.find("DataServer started") != std::string::npos);
}

void tst_logger::test_logger_levels()
{
  const auto path = getCurrentFolder();
  SystemClock clock;

  {
    Logger logger(path, clock);

    QVERIFY(logger.debug("debug"));
    QVERIFY(logger.info("info"));
    QVERIFY(logger.warning("warning"));
    QVERIFY(logger.error("error"));
  }

  const auto &logFile = findLogFile(path);

  std::ifstream stream(logFile);

  QVERIFY(stream.is_open());

  std::string line;

  QVERIFY(std::getline(stream, line));
  QVERIFY(line.find("[INFO]") != std::string::npos);
  QVERIFY(line.find("DataServer started") != std::string::npos);

  QVERIFY(std::getline(stream, line));
  QVERIFY(line.find("[DEBUG]") != std::string::npos);
  QVERIFY(line.find("debug") != std::string::npos);

  QVERIFY(std::getline(stream, line));
  QVERIFY(line.find("[INFO]") != std::string::npos);
  QVERIFY(line.find("info") != std::string::npos);

  QVERIFY(std::getline(stream, line));
  QVERIFY(line.find("[WARN]") != std::string::npos);
  QVERIFY(line.find("warning") != std::string::npos);

  QVERIFY(std::getline(stream, line));
  QVERIFY(line.find("[ERROR]") != std::string::npos);
  QVERIFY(line.find("error") != std::string::npos);

  QVERIFY(!std::getline(stream, line));
}