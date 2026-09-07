#include "tst_hardware.h"
#include "fakelcardmodule.h"
#include "lcarddatasource.h"
#include "signalmemory.h"
#include <QtCore/qtestsupport_core.h>
#include <qtestcase.h>

tst_hardware::tst_hardware() { }
tst_hardware::~tst_hardware() = default;

void tst_hardware::test_fakeLCardModule_base()
{
  using namespace qds;

  FakeLCardModule module;

  RawMemory raw;
  raw.initialize(3);

  QVERIFY(!module.read(raw.values()));

  QCOMPARE(module.readCalls, 0);

  QCOMPARE(raw.values()[0], 0.0);
  QCOMPARE(raw.values()[1], 0.0);
  QCOMPARE(raw.values()[2], 0.0);

  QVERIFY(module.start());
  QVERIFY(!module.start());

  QVERIFY(module.read(raw.values()));
  QCOMPARE(module.readCalls, 1);

  QCOMPARE(raw.values()[0], 0.0);
  QCOMPARE(raw.values()[1], 1.0);
  QCOMPARE(raw.values()[2], 2.0);

  QVERIFY(module.read(raw.values()));
  QCOMPARE(module.readCalls, 2);

  QCOMPARE(raw.values()[0], 3.0);
  QCOMPARE(raw.values()[1], 4.0);
  QCOMPARE(raw.values()[2], 5.0);

  module.stop();
  QCOMPARE(module.stopCalls, 1);

  QVERIFY(!module.read(raw.values()));
  QCOMPARE(module.readCalls, 2);

  QCOMPARE(raw.values()[0], 3.0);
  QCOMPARE(raw.values()[1], 4.0);
  QCOMPARE(raw.values()[2], 5.0);
}

void tst_hardware::test_lCardDataSource()
{
  using namespace qds;

  ModuleConfiguration configuration;
  configuration.channelCount = 3;

  auto module = std::make_unique<FakeLCardModule>();
  auto* fake = module.get();

  LCardDataSource source(
    configuration,
    std::move(module),
    std::chrono::milliseconds(1));

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
  QCOMPARE(raw.values()[1],
           raw.values()[0] + 1.0);
  QCOMPARE(raw.values()[2],
           raw.values()[0] + 2.0);

  // Ждём следующий цикл worker-а.
  QTRY_VERIFY_WITH_TIMEOUT(fake->readCalls > 1, 1000);

  QVERIFY(source.acquire(raw.values()));

  QVERIFY(raw.values()[0] >= 3.0);
  QCOMPARE(raw.values()[1],
           raw.values()[0] + 1.0);
  QCOMPARE(raw.values()[2],
           raw.values()[0] + 2.0);

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

  ModuleConfiguration configuration;
  configuration.channelCount = 3;

  auto module = std::make_unique<FakeLCardModule>();
  auto* fake = module.get();

  LCardDataSource source(
    configuration,
    std::move(module),
    std::chrono::milliseconds(1));

  RawMemory raw;
  raw.initialize(3);

  QVERIFY(source.start());
  QCOMPARE(fake->startCalls, 1u);

  for (int i = 0; i < 100; ++i)
  {
    QTRY_VERIFY_WITH_TIMEOUT(fake->readCalls > i * 3, 10);

    QVERIFY(source.acquire(raw.values()));

    QVERIFY(static_cast<int>(raw.values()[2] + 1) % 3 == 0);

    QCOMPARE(raw.values()[1],
             raw.values()[0] + 1.0);

    QCOMPARE(raw.values()[2],
             raw.values()[1] + 1.0);
  }

  source.stop();

  QCOMPARE(fake->stopCalls, 1u);
}