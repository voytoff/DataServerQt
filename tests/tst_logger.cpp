#include "tst_logger.h"
#include "fakeclock.h"
#include "systemclock.h"
#include "testsrv.h"
#include "logger.h"

#include <fstream>
#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>

tst_logger::tst_logger()
{
  const auto path = getCurrentFolder() / "logs";
  std::error_code ec;
  std::filesystem::remove_all(path, ec);
  QVERIFY(!ec);
}
tst_logger::~tst_logger() = default;

void tst_logger::test_logger_base()
{
  const auto path = getCurrentFolder() / "logs";
  SystemClock clock;
  const auto wallClockTime = clock.wallClockTime();
  const auto logFile = makeFileName(path, wallClockTime.unixMicroseconds);

  {
    Logger logger(path, clock);

    QVERIFY(logger.info("DataServer started"));
  }

  std::ifstream stream(logFile);

  QVERIFY(stream.is_open());

  std::string line;
  QVERIFY(std::getline(stream, line));

  QVERIFY(line.find("[INFO]") != std::string::npos);
  QVERIFY(line.find("DataServer started") != std::string::npos);

  stream.close();
  QVERIFY(!stream.is_open());

  std::error_code ec;
  QVERIFY(std::filesystem::remove(logFile, ec));
}

void tst_logger::test_logger_levels()
{
  const auto path = getCurrentFolder() / "logs";
  SystemClock clock;
  const auto wallClockTime = clock.wallClockTime();
  const auto logFile = makeFileName(path, wallClockTime.unixMicroseconds);

  {
    Logger logger(path, clock);

    QVERIFY(logger.debug("debug"));
    QVERIFY(logger.info("info"));
    QVERIFY(logger.warning("warning"));
    QVERIFY(logger.error("error"));
  }

  std::ifstream stream(logFile);

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

  stream.close();
  QVERIFY(!stream.is_open());

  std::error_code ec;
  QVERIFY(std::filesystem::remove(logFile, ec));
}

void tst_logger::test_logger_midnight()
{
  const auto path = getCurrentFolder() / "logs";
  FakeClock clock;

  const auto t1 =
    duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch())
        .count();

  long long mks_per_day = 24LL * 60 * 60 * 1'000'000;;
  const auto t2 = t1 + mks_per_day;


  {
    Logger logger(path, clock);

    clock.setWallClockTime(t1);
    QVERIFY(logger.info("step 1"));

    clock.setWallClockTime(t2);
    QVERIFY(logger.info("step 2"));
  }

  const auto logFile1 = makeFileName(path, t1);
  const auto logFile2 = makeFileName(path, t2);

  std::string line;

  std::ifstream stream(logFile1);

  QVERIFY(stream.is_open());

  QVERIFY(std::getline(stream, line));
  QVERIFY(line.find("[INFO]") != std::string::npos);
  QVERIFY(line.find("step 1") != std::string::npos);

  stream.close();
  QVERIFY(!stream.is_open());

  stream.open(logFile2);

  QVERIFY(stream.is_open());

  QVERIFY(std::getline(stream, line));
  QVERIFY(line.find("[INFO]") != std::string::npos);
  QVERIFY(line.find("step 2") != std::string::npos);

  stream.close();
  QVERIFY(!stream.is_open());

  std::error_code ec;
  QVERIFY(std::filesystem::remove(logFile1, ec));
  QVERIFY(std::filesystem::remove(logFile2, ec));
}

void tst_logger::test_logger_multithread()
{
  const auto path = getCurrentFolder() / "logs";
  SystemClock clock;

  const auto wallClockTime = clock.wallClockTime();
  const auto logFile =
    makeFileName(path, wallClockTime.unixMicroseconds);

  constexpr int threadCount = 8;
  constexpr int messagesPerThread = 100;
  constexpr int expectedMessages =
    threadCount * messagesPerThread;

  std::atomic<int> successfulWrites{0};

  {
    Logger logger(path, clock);

    std::vector<std::thread> threads;

    for (int i = 0; i < threadCount; ++i)
    {
      threads.emplace_back(
        [&logger, &successfulWrites, i]()
        {
          for (int j = 0; j < messagesPerThread; ++j)
          {
            if (logger.info(
                  std::format(
                    "thread={} message={}",
                    i,
                    j)))
            {
              ++successfulWrites;
            }
          }
        });
    }

    for (auto& thread : threads)
      thread.join();
  }

  QCOMPARE(
    successfulWrites.load(),
    expectedMessages);

  std::ifstream stream(logFile);

  QVERIFY(stream.is_open());

  std::string line;
  int lineCount = 0;
  std::map<int, std::vector<int>> messages;

  while (std::getline(stream, line))
  {
    QVERIFY(
      line.find("[INFO]") != std::string::npos);

    const auto threadValue =
      extract_value(line, "thread=");

    QVERIFY(threadValue);

    const auto messageValue =
      extract_value(line, "message=");

    QVERIFY(messageValue);

    messages[*threadValue].push_back(*messageValue);

    ++lineCount;
  }

  QCOMPARE(lineCount, expectedMessages);
  QCOMPARE(
    static_cast<int>(messages.size()),
    threadCount);

  for (const auto& [threadId, values] : messages)
  {
    Q_UNUSED(threadId);

    QCOMPARE(
      static_cast<int>(values.size()),
      messagesPerThread);

    auto sorted = values;
    std::ranges::sort(sorted);

    for (int i = 0; i < messagesPerThread; ++i)
      QCOMPARE(sorted[i], i);
  }

  QVERIFY(stream.eof());

  stream.close();
  QVERIFY(!stream.is_open());

  std::error_code ec;
  QVERIFY(std::filesystem::remove(logFile, ec));
}