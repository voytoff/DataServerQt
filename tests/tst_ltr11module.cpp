#include "tst_ltr11module.h"
#include <QJsonObject>
#include <qelapsedtimer.h>
#include <qtestcase.h>
#include <qtestsupport_core.h>
#include "fakeclock.h"
#include "lcarddatasource.h"
#include "ltr11configurationbuilder.h"
#include "ltr11configurationfactory.h"
#include "ltr11configurationvalidator.h"
#include "ltr11module.h"
#include "signalmemory.h"
#include "testsrv.h"

#include <limits>
#include <cmath>


tst_ltr11module::tst_ltr11module() { }
tst_ltr11module::~tst_ltr11module() = default;

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
  FakeClock clock;


  LCardDataSource source(
    ModuleId{0},
    config.channels.size(),
    std::move(module),
    clock);

  QVERIFY(!source.isRunning());

  QVERIFY(source.start());
  QVERIFY(source.isRunning());

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
  QVERIFY(!source.isRunning());

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

    const LCardReadResult ok =
      ltr11->readBlock(raw.values());

    //const qint64 after = timer.elapsed();
    const double durationMs =
      static_cast<double>(readTimer.nsecsElapsed()) /
      1'000'000.0;

    if (!ok.frameCount)
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


  auto module = std::make_unique<Ltr11Module>(config);
  auto* ltr11 = module.get();

  RawMemory raw;
  raw.initialize(
    config.channels.size() *
    ltr11->blockFrameCapacity());

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

    const LCardReadResult ok =
      ltr11->readBlock(raw.values());

    const double durationMs =
      static_cast<double>(
        readTimer.nsecsElapsed()) /
      1'000'000.0;

    totalReadMs += durationMs;
    maxReadMs =
      std::max(maxReadMs, durationMs);

    if (ok.frameCount)
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
  FakeClock clock;

  LCardDataSource source(
    ModuleId{0},
    config.channels.size(),
    std::move(module),
    clock);

  QVERIFY(!source.isRunning());

  QVERIFY(source.start());
  QVERIFY(source.isRunning());

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
  QVERIFY(!source.isRunning());
}

void tst_ltr11module::test_ltr11configurationvalidator()
{
  Ltr11Configuration cfg{
    .address = QHostAddress("192.168.10.12").toIPv4Address(),
    .port = 11111,
    .crateSerial = "CRATE16/EU",
    .slot = 11,
    .mode = 0,
    .channelRate = 1000,
  };

  // Пустой список каналов
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.channels.push_back(
    {
      .channel = 0,
      .range = 0,
    });

  // Корректная конфигурация
  QVERIFY(Ltr11ConfigurationValidator::validate(cfg));

  for (uint8_t i = 1; i < 17; ++i) {
    cfg.channels.push_back(
      {
        .channel = i,
        .range = static_cast<uint8_t>(i % 4),
      });
  }

  QCOMPARE(cfg.channels.size(), 17);

  // Более 16 логических каналов при mode = 0
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.channels.pop_back(); // 17 канал

  QVERIFY(Ltr11ConfigurationValidator::validate(cfg));

  cfg.channels[15].channel = 16;

  // Проверка канала 16 при дифференциальном режиме
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.channels[15].channel = 15;

  for (uint8_t i = 16; i < 33; ++i) {
    cfg.channels.push_back(
      {
        .channel = i,
        .range = static_cast<uint8_t>(i % 4),
      });
  }

  QCOMPARE(cfg.channels.size(), 33);

  cfg.mode = 1;

  // Более 32 логических каналов при mode 1
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.channels.pop_back(); // 33 канал

  QCOMPARE(cfg.channels.size(), 32);

  QVERIFY(Ltr11ConfigurationValidator::validate(cfg));

  cfg.channels[0].channel = 32;

  // Физический канал с номером 32
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.channels[0].channel = 30;
  QCOMPARE(cfg.channels[30].channel, 30);

  // Проверка повторяющихся физических каналов 30
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.channels.erase(cfg.channels.begin());

  QVERIFY(Ltr11ConfigurationValidator::validate(cfg));

  cfg.mode = 2;

  // Недопустимый mode >= 2
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.mode = 1;

  QVERIFY(Ltr11ConfigurationValidator::validate(cfg));

  cfg.channels[0].range = 4;

  // Недопустимый range >= 4
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.channels.erase(cfg.channels.begin());

  QVERIFY(Ltr11ConfigurationValidator::validate(cfg));

  cfg.channelRate = 0.0;

  // Нулевая частота
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.channelRate = -100.0;

  // Отрицательная частота
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.channelRate = 400'000.0 / cfg.channels.size();

  QVERIFY(Ltr11ConfigurationValidator::validate(cfg));

  cfg.channelRate += 0.1;

  // Частота АЦП выше 400 кГц
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.channelRate = std::numeric_limits<double>::quiet_NaN();

  // Частота АЦП NaN
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.channelRate =  std::numeric_limits<double>::infinity();

  // Частота АЦП бесконечная
  QVERIFY(!Ltr11ConfigurationValidator::validate(cfg));

  cfg.channelRate = 100.0;

  // Корректная конфигурация
  QVERIFY(Ltr11ConfigurationValidator::validate(cfg));


  // Проверка валидатора канала
  Ltr11ChannelConfiguration channel{
    .channel = 31,
    .range = 3,
  };

  // Корректный канал
  QVERIFY(Ltr11ConfigurationValidator::validate(channel));

  channel.channel = 32;
  // Физический канал с номером 32
  QVERIFY(!Ltr11ConfigurationValidator::validate(channel));

  channel.channel = 31;

  channel.range = 4;
  // Недопустимый range >= 4
  QVERIFY(!Ltr11ConfigurationValidator::validate(channel));
}

void tst_ltr11module::test_Ltr11ConfigurationFactory()
{
  ModuleRuntimeConfiguration cfg{
    .module = {
      .id = ModuleId{1},
      .serial = "MODULE11",
      .crate = CrateId{1},
      .slot = 12,
      .type = ModuleType::LTR11,
      .description = "Тестовый модуль",
    },
    .crate = CrateInfo{
      .id = CrateId{1},
      .serial = "CRATE16",
      .host = "192.168.1.10",
      .port = 11111,
      .description = "Тестовый крейт"
    },
    .configuration = {
      .configurationId = ConfigurationId{1},
      .module = ModuleId{1},
      .settings = QJsonObject{
        {"mode", 0}
      }
    },
  };
  cfg.tags.push_back({
    .tag = TagId{116},
    .module = ModuleId{1},
    .channel = ChannelId{0},
    .settings = QJsonObject{
      {"range", 1}
    }
  });

  Ltr11ConfigurationFactory factory;

  const auto& configuration = factory.create(cfg);
}
