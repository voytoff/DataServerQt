#include "tst_hardware.h"
#include "fakeclock.h"
#include "fakedatablocksink.h"
#include "fakelcardmodule.h"
#include "lcarddatasource.h"
#include "ltr11configurationbuilder.h"
#include "moduleruntimeconfiguration.h"
#include "smartblocklcardmodule.h"
#include "signalmemory.h"
#include <QtCore/qtestsupport_core.h>
#include <qtestcase.h>
#include <QJsonObject>
#include <QHostAddress>

tst_hardware::tst_hardware() { }
tst_hardware::~tst_hardware() = default;

void tst_hardware::test_fakeLCardModule_base()
{
  using namespace qds;

  FakeLCardModule module(4, 3);

  RawMemory raw;
  raw.initialize(4 * 3);
  const auto &values = raw.values();

  std::size_t frameCount = module.readBlock(raw.values());

  QCOMPARE(module.readCalls, 0);
  QCOMPARE(frameCount, std::size_t{0});

  QCOMPARE(values[0], 0.0);
  QCOMPARE(values[1], 0.0);
  QCOMPARE(values[2], 0.0);
  QCOMPARE(values[4], 0.0);

  QCOMPARE(values[4], 0.0);
  QCOMPARE(values[5], 0.0);
  QCOMPARE(values[6], 0.0);
  QCOMPARE(values[7], 0.0);

  QCOMPARE(values[8], 0.0);
  QCOMPARE(values[9], 0.0);
  QCOMPARE(values[10], 0.0);
  QCOMPARE(values[11], 0.0);

  QVERIFY(module.start());
  QVERIFY(!module.start());

  frameCount = module.readBlock(raw.values());

  QCOMPARE(module.readCalls, 1);
  QCOMPARE(frameCount, std::size_t{3});

  QCOMPARE(values[0], 0.0);
  QCOMPARE(values[1], 1.0);
  QCOMPARE(values[2], 2.0);
  QCOMPARE(values[3], 3.0);

  QCOMPARE(values[4], 4.0);
  QCOMPARE(values[5], 5.0);
  QCOMPARE(values[6], 6.0);
  QCOMPARE(values[7], 7.0);

  QCOMPARE(values[8], 8.0);
  QCOMPARE(values[9], 9.0);
  QCOMPARE(values[10], 10.0);
  QCOMPARE(values[11], 11.0);

  frameCount = module.readBlock(raw.values());

  QCOMPARE(module.readCalls, 2);
  QCOMPARE(frameCount, std::size_t{3});

  QCOMPARE(values[0], 12.0);
  QCOMPARE(values[1], 13.0);
  QCOMPARE(values[2], 14.0);
  QCOMPARE(values[3], 15.0);

  QCOMPARE(values[4], 16.0);
  QCOMPARE(values[5], 17.0);
  QCOMPARE(values[6], 18.0);
  QCOMPARE(values[7], 19.0);

  QCOMPARE(values[8], 20.0);
  QCOMPARE(values[9], 21.0);
  QCOMPARE(values[10], 22.0);
  QCOMPARE(values[11], 23.0);

  module.stop();
  QCOMPARE(module.stopCalls, 1);

  frameCount = module.readBlock(raw.values());

  QCOMPARE(module.readCalls, 2);
  QCOMPARE(frameCount, std::size_t{0});

  QCOMPARE(values[0], 12.0);
  QCOMPARE(values[1], 13.0);
  QCOMPARE(values[2], 14.0);
  QCOMPARE(values[3], 15.0);

  QCOMPARE(values[4], 16.0);
  QCOMPARE(values[5], 17.0);
  QCOMPARE(values[6], 18.0);
  QCOMPARE(values[7], 19.0);

  QCOMPARE(values[8], 20.0);
  QCOMPARE(values[9], 21.0);
  QCOMPARE(values[10], 22.0);
  QCOMPARE(values[11], 23.0);
}

void tst_hardware::test_lCardDataSource()
{
  using namespace qds;

  auto module = std::make_unique<FakeLCardModule>(3, 3);
  auto* fake = module.get();

  FakeClock clock;

  LCardDataSource source(
    ModuleId{0},
    3,
    std::move(module),
    clock);

  RawMemory raw;
  raw.initialize(3);

  // До start() данных нет.
  QVERIFY(!source.acquire(raw.values()));

  QCOMPARE(fake->startCalls, 0u);
  QCOMPARE(fake->stopCalls, 0u);
  QCOMPARE(fake->readCalls, 0u);

  // Запускаем DataSource.
  QVERIFY(source.start());

  QCOMPARE(fake->startCalls, 1u);

  // Повторный start() ничего не запускает.
  QVERIFY(source.start());

  QCOMPARE(fake->startCalls, 1u);

  // Ждём, пока worker выполнит хотя бы один read().
  QTRY_VERIFY_WITH_TIMEOUT(fake->readCalls > 0, 1000);

  // Теперь acquire() должен получить данные.
  QVERIFY(source.acquire(raw.values()));

  QVERIFY(raw.values()[0] >= 0.0);
  QCOMPARE(raw.values()[1], raw.values()[0] + 1.0);
  QCOMPARE(raw.values()[2], raw.values()[0] + 2.0);

  // Ждём следующий цикл worker-а.
  QTRY_VERIFY_WITH_TIMEOUT(fake->readCalls > 1, 1000);

  QVERIFY(source.acquire(raw.values()));

  QVERIFY(raw.values()[0] >= 3.0);
  QCOMPARE(raw.values()[1], raw.values()[0] + 1.0);
  QCOMPARE(raw.values()[2], raw.values()[0] + 2.0);

  // Запоминаем последние данные.
  const auto value0 = raw.values()[0];
  const auto value1 = raw.values()[1];
  const auto value2 = raw.values()[2];

  // Останавливаем DataSource.
  source.stop();

  QCOMPARE(fake->stopCalls, 1u);

  // Повторный stop() не должен повторно останавливать модуль.
  source.stop();

  QCOMPARE(fake->stopCalls, 1u);

  // После stop() acquire() запрещён.
  QVERIFY(!source.acquire(raw.values()));

  // Буфер пользователя не должен измениться.
  QCOMPARE(raw.values()[0], value0);
  QCOMPARE(raw.values()[1], value1);
  QCOMPARE(raw.values()[2], value2);
}

void tst_hardware::test_lCardDataSource_data_integrity()
{
  using namespace qds;

  auto module = std::make_unique<FakeLCardModule>(3, 10);
  auto* fake = module.get();

  FakeClock clock;

  LCardDataSource source(
    ModuleId{0},
    3,
    std::move(module),
    clock);

  RawMemory raw;
  raw.initialize(3);

  QVERIFY(source.start());
  QCOMPARE(fake->startCalls, 1u);

  for (int i = 0; i < 100; ++i)
  {
    QTRY_VERIFY_WITH_TIMEOUT(fake->readCalls > i * 3, 10);

    QVERIFY(source.acquire(raw.values()));

    QVERIFY(static_cast<int>(raw.values()[2] + 1) % 3 == 0);

    QCOMPARE(raw.values()[1], raw.values()[0] + 1.0);

    QCOMPARE(raw.values()[2], raw.values()[1] + 1.0);
  }

  source.stop();

  QCOMPARE(fake->stopCalls, 1u);
}

void tst_hardware::test_ltr11configurationbuilder()
{
  using namespace qds;

  Ltr11Configuration config;

  CrateInfo crate
  {
    .id = {0},
    .serial = "LTR1101",
    .type = CrateType::LTR_EU_16_1,
    .host = "127.0.0.1",
    .port = 11111
  };
  ModuleRuntimeConfiguration cfg
  {
    .module = {
      .id = {0},
      .serial = "12340987",
      .crate = {0},
      .slot = 1,
      .type = ModuleType::LTR11
    },
    .crate = crate,
    .configuration = {
      .configurationId = {1},
      .module = {0},
      .settings = QJsonDocument::fromJson(R"({})").object()
    },
    .tags = {
      {TagId{1}, ModuleId{0}, ChannelId{0}, QJsonDocument::fromJson(R"({"mode":1,"range":0})").object()},
      {TagId{5}, ModuleId{0}, ChannelId{12}, QJsonDocument::fromJson(R"({"mode":2,"range":1})").object()},
    }
  };

  Ltr11ConfigurationBuilder builder;

  QVERIFY(builder.build(cfg, config));

  QCOMPARE(config.address, QHostAddress("127.0.0.1").toIPv4Address());
  QCOMPARE(config.channelRate, 1000);
  QCOMPARE(config.crateSerial, "LTR1101");
  QCOMPARE(config.port, 11111);
  QCOMPARE(config.slot, 1);

  const auto &channels = config.channels;
  QCOMPARE(channels.size(), 2);

  const auto &channel0 = channels[0];
  QCOMPARE(channel0.channel, 0);
  QCOMPARE(channel0.mode, 1);
  QCOMPARE(channel0.range, 0);

  const auto &channel1 = channels[1];
  QCOMPARE(channel1.channel, 12);
  QCOMPARE(channel1.mode, 2);
  QCOMPARE(channel1.range, 1);

  const auto validConfiguration = config;

  cfg.tags.push_back(
    {TagId{9}, ModuleId{0}, ChannelId{17}, QJsonDocument::fromJson(R"({"range":2})").object()}
    );
  QCOMPARE(cfg.tags.size(), 3);

  QVERIFY(!builder.build(cfg, config));

  cfg.tags.pop_back();

  cfg.tags.push_back(
    {TagId{9}, ModuleId{0}, ChannelId{17}, QJsonDocument::fromJson(R"({"mode":2})").object()}
    );
  QCOMPARE(cfg.tags.size(), 3);

  QVERIFY(!builder.build(cfg, config));

  cfg.tags.pop_back();

  cfg.tags.push_back(
    {TagId{9}, ModuleId{0}, ChannelId{17},
     QJsonDocument::fromJson(R"({})").object()}
    );

  QVERIFY(!builder.build(cfg, config));

  cfg.tags.pop_back();

  cfg.tags.push_back(
    {TagId{9}, ModuleId{0}, ChannelId{17}, QJsonDocument::fromJson(R"({"mode":3,"range":1})").object()}
    );
  QCOMPARE(cfg.tags.size(), 3);

  QVERIFY(!builder.build(cfg, config));

  cfg.tags.pop_back();

  cfg.tags.push_back(
    {TagId{9}, ModuleId{0}, ChannelId{17}, QJsonDocument::fromJson(R"({"mode":1,"range":4})").object()}
    );
  QCOMPARE(cfg.tags.size(), 3);

  QVERIFY(!builder.build(cfg, config));

  QCOMPARE(config.address, validConfiguration.address);
  QCOMPARE(config.channels.size(), validConfiguration.channels.size());


  cfg.tags.pop_back();
  QCOMPARE(cfg.tags.size(), 2);
  QVERIFY(builder.build(cfg, config));


  cfg.crate.host = "127.0.256.1";
  QVERIFY(!builder.build(cfg, config));

  cfg.crate.host = "";
  QVERIFY(builder.build(cfg, config));

  cfg.tags.clear();
  QVERIFY(!builder.build(cfg, config));
}

void tst_hardware::test_acquire_returns_last_frame()
{
  using namespace qds;

  constexpr std::size_t ChannelCount = 4;
  constexpr std::size_t FrameCount = 3;

  auto module =
    std::make_unique<SmartBlockLCardModule>(3, 4, 1);

  FakeClock clock;

  LCardDataSource source(
    ModuleId{0},
    ChannelCount,
    std::move(module),
    clock);

  QVERIFY(source.start());

  QTest::qWait(10);

  std::array<double, ChannelCount> values{};

  QVERIFY(source.acquire(values));

  source.stop();

  QCOMPARE(values[0], 8.0);
  QCOMPARE(values[1], 9.0);
  QCOMPARE(values[2], 10.0);
  QCOMPARE(values[3], 11.0);
}

void tst_hardware::test_lCardDataSource_fake_push_archive()
{
  using namespace qds;

  constexpr std::size_t ChannelCount = 3;

  auto module = std::make_unique<SmartBlockLCardModule>(3, 3, 1);

  FakeDataBlockSink dataSink;
  FakeClock clock;

  LCardDataSource source(
    ModuleId{0},
    ChannelCount,
    std::move(module),
    clock,
    &dataSink);

  RawMemory raw;
  raw.initialize(ChannelCount);

  QVERIFY(source.start());

  QTest::qWait(10);
  QVERIFY(source.acquire(raw.values()));
  source.stop();

  QCOMPARE(dataSink.m_channelCount, std::size_t{3});
  QCOMPARE(dataSink.m_frameCount, std::size_t{3});
  QCOMPARE(dataSink.m_values.size(), std::size_t{9});
  QCOMPARE(dataSink.m_firstFrameIndex, 0);

  QCOMPARE(dataSink.m_values[0], 0.0);
  QCOMPARE(dataSink.m_values[1], 1.0);
  QCOMPARE(dataSink.m_values[2], 2.0);
  QCOMPARE(dataSink.m_values[3], 3.0);
  QCOMPARE(dataSink.m_values[4], 4.0);
  QCOMPARE(dataSink.m_values[5], 5.0);
  QCOMPARE(dataSink.m_values[6], 6.0);
  QCOMPARE(dataSink.m_values[7], 7.0);
  QCOMPARE(dataSink.m_values[8], 8.0);

  QCOMPARE(raw.values()[0], 6.0);
  QCOMPARE(raw.values()[1], 7.0);
  QCOMPARE(raw.values()[2], 8.0);
}
