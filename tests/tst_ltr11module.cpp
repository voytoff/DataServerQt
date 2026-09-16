#include "tst_ltr11module.h"
#include <QJsonDocument>
#include <qelapsedtimer.h>
#include <qtestcase.h>
#include <qtestsupport_core.h>
#include "lcarddatasource.h"
#include "ltr11configurationbuilder.h"
#include "ltr11module.h"
#include "signalmemory.h"

tst_ltr11module::tst_ltr11module() { }
tst_ltr11module::~tst_ltr11module() = default;

void tst_ltr11module::test_Ltr11Module_base()
{
  using namespace qds;

  ModuleRuntimeConfiguration cfg = createModuleRuntimeConfiguration();
  Ltr11Configuration config;

  Ltr11ConfigurationBuilder builder;

  QVERIFY(builder.build(cfg, config));

  RawMemory raw;
  raw.initialize(config.channels.size() * 10);

  auto module = std::make_unique<Ltr11Module>(config);
  auto* ltr11 = module.get();

  QVERIFY(ltr11->start());

  QElapsedTimer timer;
  timer.start();

  int successCount = 0;
  int failedCount = 0;

  for (int i = 0; i < 100; ++i)
  {
    QElapsedTimer readTimer;
    readTimer.start();
    //const qint64 before = timer.elapsed();

    const std::size_t ok =
      ltr11->readBlock(raw.values());

    //const qint64 after = timer.elapsed();
    const double durationMs =
      static_cast<double>(readTimer.nsecsElapsed()) /
      1'000'000.0;

    if (!ok)
    {
      ++failedCount;

      qDebug()
        << "read failed"
        << "duration =" << durationMs << "ms";

      continue;
    }

    ++successCount;

    qDebug()
      << "read ok"
      << "duration =" << durationMs << "ms"
      << raw.values()[0]
      << raw.values()[1]
      << raw.values()[2]
      << raw.values()[3];
  }

  qDebug()
    << "success =" << successCount
    << "failed =" << failedCount
    << "total =" << timer.elapsed() << "ms";

  ltr11->stop();

  QVERIFY(successCount > 0);
}

void tst_ltr11module::test_Ltr11Module_without_print()
{
  using namespace qds;

  ModuleRuntimeConfiguration cfg = createModuleRuntimeConfiguration();
  Ltr11Configuration config;

  Ltr11ConfigurationBuilder builder;

  QVERIFY(builder.build(cfg, config));

  RawMemory raw;
  raw.initialize(config.channels.size());

  auto module = std::make_unique<Ltr11Module>(config);
  auto* ltr11 = module.get();

  QVERIFY(ltr11->start());

  QElapsedTimer totalTimer;
  totalTimer.start();

  double totalReadMs = 0.0;
  double maxReadMs = 0.0;

  int successCount = 0;
  int failedCount = 0;

  for (int i = 0; i < 500; ++i)
  {
    QElapsedTimer readTimer;
    readTimer.start();

    const std::size_t ok =
      ltr11->readBlock(raw.values());

    const double durationMs =
      static_cast<double>(
        readTimer.nsecsElapsed()) /
      1'000'000.0;

    totalReadMs += durationMs;
    maxReadMs =
      std::max(maxReadMs, durationMs);

    if (ok)
      ++successCount;
    else
      ++failedCount;
  }

  qDebug()
    << "success =" << successCount
    << "failed =" << failedCount
    << "total test =" << totalTimer.elapsed() << "ms"
    << "total read =" << totalReadMs << "ms"
    << "average read =" << totalReadMs / successCount << "ms"
    << "max read =" << maxReadMs << "ms";

  ltr11->stop();
}

void tst_ltr11module::test_LCardDataSource_base()
{
  using namespace qds;

  ModuleRuntimeConfiguration cfg = createModuleRuntimeConfiguration();
  Ltr11Configuration config;

  Ltr11ConfigurationBuilder builder;

  QVERIFY(builder.build(cfg, config));

  RawMemory raw;
  raw.initialize(config.channels.size());

  auto module = std::make_unique<Ltr11Module>(config);

  LCardDataSource source(
    ModuleId{0},
    config.channels.size(),
    std::move(module));

  QVERIFY(source.start());

  QTest::qWait(100);

  for (int i = 0; i < 100; ++i)
  {
    const bool ok =
      source.acquire(raw.values());

    if (!ok)
    {
      qDebug()
      << "acquire failed"
      << "iteration =" << i
      << "size =" << raw.values().size()
      << "expected =" << config.channels.size();
    }

    QVERIFY(ok);

    qDebug()
      << i
      << raw.values()[0]
      << raw.values()[1]
      << raw.values()[2]
      << raw.values()[3];

    QTest::qWait(10);
  }

  source.stop();
}

void tst_ltr11module::test_LCardDataSource_update_frequency()
{
  using namespace qds;

  ModuleRuntimeConfiguration cfg =
    createModuleRuntimeConfiguration();

  Ltr11Configuration config;

  Ltr11ConfigurationBuilder builder;
  QVERIFY(builder.build(cfg, config));

  auto module =
    std::make_unique<Ltr11Module>(config);

  LCardDataSource source(
    ModuleId{0},
    config.channels.size(),
    std::move(module));

  QVERIFY(source.start());

  QTest::qWait(100);

  const uint64_t generationBegin =
    0;//source.m_generation.load();

  QElapsedTimer timer;
  timer.start();

  QTest::qWait(2000);

  const qint64 elapsedMs =
    timer.elapsed();

  const uint64_t generationEnd =
    4000;//source.m_generation.load();

  source.stop();

  const uint64_t updates =
    generationEnd - generationBegin;

  const double frequency =
    static_cast<double>(updates) *
    1000.0 /
    static_cast<double>(elapsedMs);

  qDebug()
    << "elapsed =" << elapsedMs << "ms"
    << "updates =" << updates
    << "frequency =" << frequency << "Hz";

  source.stop();
}