#include "tst_logger.h"
#include "logger.h"
#include "testsrv.h"
#include <fstream>

tst_logger::tst_logger() { }
tst_logger::~tst_logger() = default;

void tst_logger::test_logger_base()
{
  const auto path = getFilePath("file.log");

  {
    Logger logger(path);

    QVERIFY(logger.info("DataServer started"));
  }

  std::ifstream stream(path);

  QVERIFY(stream.is_open());

  std::string line;
  QVERIFY(std::getline(stream, line));

  QVERIFY(line.find("[INFO]") != std::string::npos);
  QVERIFY(line.find("DataServer started") != std::string::npos);
}

void tst_logger::test_logger_levels()
{
  const auto path = getFilePath("file.log");

  {
    Logger logger(path);

    QVERIFY(logger.debug("debug"));
    QVERIFY(logger.info("info"));
    QVERIFY(logger.warning("warning"));
    QVERIFY(logger.error("error"));
  }

  std::ifstream stream(path);

  QVERIFY(stream.is_open());

  std::string line;

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