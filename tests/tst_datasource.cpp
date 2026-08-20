#include "tst_datasource.h"
#include "buffermanager.h"
#include "datasourcefactory.h"
#include "datasourcemanager.h"
#include "failingdatasource.h"
#include "fakedatasource.h"
#include "fakelcardmodule.h"
#include "generatordatasource.h"
#include "hardwaremodulefactory.h"
#include "testsrv.h"
#include <qtestcase.h>
#include <qtestsupport_core.h>
#include "systemclock.h"
#include "QPointer"

tst_datasource::tst_datasource() { }
tst_datasource::~tst_datasource() = default;
/*
void tst_datasource::test_generatorDataSource_once()
{
  using namespace qds;
  // создаем конфигурацию
  constexpr TagId tags[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags, std::size(tags));
  SystemClock clock;

  TestSrv srv(cfg);

  GeneratorDataSource source(srv.storage, cfg, clock);
  QVERIFY(source.start());
  QVERIFY(source.isRunning());
  QVERIFY(!source.start());
  QVERIFY(source.isRunning());

  source.stop();
  QVERIFY(!source.isRunning());

  uint64_t t(1234567);
  QVERIFY(source.generateOnce(t));

  Sample sample;
  QCOMPARE(srv.storage.sample(tags[0]).value, 0.f);
  QCOMPARE(srv.storage.sample(tags[1]).value, 1.f);

  QCOMPARE(srv.storage.timestamp(tags[0]), t);
  QCOMPARE(srv.storage.timestamp(tags[1]), t);

  uint64_t t2 = 7777777;

  QVERIFY(source.generateOnce(t2));

  QCOMPARE(srv.storage.sample(tags[0]).value, 2.f);
  QCOMPARE(srv.storage.sample(tags[1]).value, 3.f);

  QCOMPARE(srv.storage.timestamp(tags[0]), t2);
  QCOMPARE(srv.storage.timestamp(tags[1]), t2);
}

void tst_datasource::test_generatorDataSource_onceTwoModule()
{
  using namespace qds;
  // создаем конфигурацию
  std::vector<std::vector<TagId>> modules = {
    {{0},{1}},
    {{2},{3},{4}}
  };
  SystemConfiguration cfg = createTestConfig(modules);
  SystemClock clock;

  TestSrv srv(cfg);

  GeneratorDataSource source(srv.storage, cfg, clock);

  // ---------- First generation ----------
  uint64_t t(1234567);
  QVERIFY(source.generateOnce(t));

  Sample sample;

  // проверки
  const auto& tags = cfg.moduleTags({0}); // 2 тега
  QCOMPARE(srv.storage.sample(tags[0]).value, 0.f);
  QCOMPARE(srv.storage.sample(tags[1]).value, 1.f);

  QCOMPARE(srv.storage.timestamp(tags[0]), t);
  QCOMPARE(srv.storage.timestamp(tags[1]), t);

  const auto& tags2 = cfg.moduleTags({1}); // 3 тега
  QCOMPARE(srv.storage.sample(tags2[0]).value, 0.f);
  QCOMPARE(srv.storage.sample(tags2[1]).value, 1.f);
  QCOMPARE(srv.storage.sample(tags2[2]).value, 2.f);

  QCOMPARE(srv.storage.timestamp(tags2[0]), t);
  QCOMPARE(srv.storage.timestamp(tags2[1]), t);
  QCOMPARE(srv.storage.timestamp(tags2[2]), t);

  QCOMPARE(srv.storage.moduleTimestamp(cfg.modules()[0].id), t);
  QCOMPARE(srv.storage.moduleTimestamp(cfg.modules()[1].id), t);

  // ---------- Second generation ----------
  uint64_t t2 = 9999999;
  QVERIFY(source.generateOnce(t2));

  // проверки
  QCOMPARE(srv.storage.sample(tags[0]).value, 2.f);
  QCOMPARE(srv.storage.sample(tags[1]).value, 3.f);

  QCOMPARE(srv.storage.timestamp(tags[0]), t2);
  QCOMPARE(srv.storage.timestamp(tags[1]), t2);

  QCOMPARE(srv.storage.sample(tags2[0]).value, 3.f);
  QCOMPARE(srv.storage.sample(tags2[1]).value, 4.f);
  QCOMPARE(srv.storage.sample(tags2[2]).value, 5.f);

  QCOMPARE(srv.storage.timestamp(tags2[0]), t2);
  QCOMPARE(srv.storage.timestamp(tags2[1]), t2);
  QCOMPARE(srv.storage.timestamp(tags2[2]), t2);

  QCOMPARE(srv.storage.moduleTimestamp(cfg.modules()[0].id), t2);
  QCOMPARE(srv.storage.moduleTimestamp(cfg.modules()[1].id), t2);
}

void tst_datasource::test_generatorDataSource_periodicCall()
{
  using namespace qds;
  constexpr TagId tags[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags, std::size(tags));
  SystemClock clock;

  TestSrv srv(cfg);

  GeneratorDataSource source(srv.storage, cfg, clock);

  PeriodicDataSourceRunner runner(source);
  runner.start(10);
  QVERIFY(source.isRunning());

  QTest::qWait(1000);

  Sample beforeSample;
  beforeSample = srv.storage.sample(tags[0]);
  QVERIFY(beforeSample.value > 0.f);
  qDebug() << "тег 0 before:" << beforeSample.value;

  beforeSample = srv.storage.sample(tags[1]);
  QVERIFY(beforeSample.value > 0.f);
  qDebug() << "тег 1 before:" << beforeSample.value;

  //QVERIFY(srv.storage.timestamp(tags[0]) > 0);
  //QVERIFY(srv.storage.timestamp(tags[1]) > 0);

  const auto beforeCount = source.generationCount();

  QVERIFY(beforeCount >= 60);
  QVERIFY(beforeCount <= 90);

  uint64_t t1 = srv.storage.moduleTimestamp({0});

  qDebug() << "generationCount before" << beforeCount;

  QTest::qWait(100);

  Sample afterSample;

  afterSample = srv.storage.sample(tags[0]);
  QVERIFY(afterSample.value > beforeSample.value);
  qDebug() << "тег 0 after:" << afterSample.value;

  afterSample = srv.storage.sample(tags[1]);
  QVERIFY(afterSample.value > beforeSample.value);
  qDebug() << "  тег 1 after:" << afterSample.value;

  const auto afterCount = source.generationCount();

  QVERIFY(afterCount >= 70);
  QVERIFY(afterCount <= 100);

  qDebug() << "generationCount after" << afterCount;
  qDebug() << "time:" << srv.storage.moduleTimestamp({0}) - t1;

  const auto delta = afterCount - beforeCount;

  QVERIFY(delta >= 5);
  QVERIFY(delta <= 12);

  runner.stop();
  QVERIFY(!source.isRunning());

  // проверка после остановки
  Sample s1, s2;
  const auto count = source.generationCount();
  s1 = srv.storage.sample(tags[0]);

  QTest::qWait(100);

  QCOMPARE(source.generationCount(), count);
  s2 = srv.storage.sample(tags[0]);

  QCOMPARE(s1.value, s2.value);
}

void tst_datasource::test_dataSourceManager_withoutSources()
{
  using namespace qds;
  DataSourceManager manager;

  QCOMPARE(manager.size(), std::size_t(0));

  QVERIFY(manager.start());
  QVERIFY(!manager.start());

  QVERIFY(manager.isRunning());
  QVERIFY(manager.step());

  manager.stop();

  QVERIFY(!manager.isRunning());
  QVERIFY(!manager.step());

  QCOMPARE(manager.size(), std::size_t(0));

  QVERIFY(manager.start());
  QVERIFY(manager.isRunning());

  manager.stop();

  QVERIFY(!manager.isRunning());
}

void tst_datasource::test_dataSourceManager_withFakeSource()
{
  using namespace qds;
  auto ptr = std::make_unique<FakeActiveDataSource>();
  auto source = ptr.get();

  DataSourceManager manager;

  manager.add(std::move(ptr));

  QVERIFY(!source->isRunning());

  QVERIFY(manager.start());

  QVERIFY(source->isRunning());

  QVERIFY(manager.step());

  manager.stop();

  QVERIFY(!source->isRunning());

  QCOMPARE(source->startCalls, 1);
  QCOMPARE(source->stepCalls, 1);
  QCOMPARE(source->stopCalls, 1);
}
*/
void tst_datasource::test_hardwareFactory_createFake()
{
  using namespace qds;

  HardwareModuleFactory factory;

  ModuleInfo module;
  module.type = ModuleType::Fake;

  auto device = factory.create(module);

  QVERIFY(device != nullptr);

  auto* fake = dynamic_cast<FakeLCardModule*>(device.get());

  QVERIFY(fake != nullptr);
}

void tst_datasource::test_hardwareFactory_unknownType()
{
  using namespace qds;

  HardwareModuleFactory factory;

  ModuleInfo module;
  module.type = ModuleType::Unknown;

  auto device = factory.create(module);

  QVERIFY(device == nullptr);
}

void tst_datasource::test_dataSourceFactory_registerType_create()
{
  using namespace qds;
  DataSourceFactory factory;
  QVERIFY(factory.registerType(
    ModuleType::Fake,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FakeDataSource>(
        cfg.settings);
    }));

  QJsonObject jsonObj;
  jsonObj.insert("name", "Fake");
  jsonObj.insert("frequency", 100);
  ModuleConfiguration cfg{.module = {.type = ModuleType::Fake}, .settings = jsonObj};

  auto source = factory.create(cfg);
  QVERIFY(source != nullptr);

  auto *fake = static_cast<FakeDataSource*>(source.get());

  QCOMPARE(fake->m_settings["name"], "Fake");
  QCOMPARE(fake->m_settings["frequency"], 100);

  // повторная регистрация
  QVERIFY(!factory.registerType(
    ModuleType::Fake,
    [](const ModuleConfiguration&)
    {
      return std::make_unique<FakeDataSource>();
    }));

  // Unknown
  ModuleConfiguration unknown;
  unknown.module.type = ModuleType::Unknown;

  QVERIFY(factory.create(unknown) == nullptr);

  // незарегистрированный тип
  ModuleConfiguration unregistered;
  unregistered.module.type = ModuleType::LCard;

  QVERIFY(factory.create(unregistered) == nullptr);
}

void tst_datasource::test_datasource_layout()
{
  using namespace qds;
  SystemConfiguration sc = createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(sc);

  QCOMPARE(layout.rawSignalCount(), 2u);
  QCOMPARE(layout.rawOffset(ModuleId{0}), 0u);

  auto raw0 = layout.location(SignalId{0});
  QCOMPARE(raw0.area, SignalMemoryArea::Raw);
  QCOMPARE(raw0.index, 0u);

  auto raw1 = layout.location(SignalId{1});
  QCOMPARE(raw1.area, SignalMemoryArea::Raw);
  QCOMPARE(raw1.index, 1u);
}

void tst_datasource::test_datasource_layout_someModules()
{
  using namespace qds;
  SystemConfiguration sc = createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(sc);

  QCOMPARE(layout.rawSignalCount(), 7u);
  QCOMPARE(layout.rawOffset(ModuleId{0}), 0u);
  QCOMPARE(layout.rawOffset(ModuleId{1}), 2u);
  QCOMPARE(layout.rawOffset(ModuleId{2}), 5u);

  auto raw0 = layout.location(SignalId{0});
  QCOMPARE(raw0.index, 0u);

  auto raw1 = layout.location(SignalId{1});
  QCOMPARE(raw1.index, 1u);

  auto raw2 = layout.location(SignalId{2});
  QCOMPARE(raw2.index, 2u);

  auto raw3 = layout.location(SignalId{3});
  QCOMPARE(raw3.index, 3u);

  auto raw4 = layout.location(SignalId{4});
  QCOMPARE(raw4.index, 4u);

  auto raw5 = layout.location(SignalId{5});
  QCOMPARE(raw5.index, 5u);

  auto raw6 = layout.location(SignalId{6});
  QCOMPARE(raw6.index, 6u);
}

void tst_datasource::test_datasource_layout_raw_calculated()
{
  using namespace qds;
  SystemConfiguration sc = createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(sc);

  QCOMPARE(layout.rawSignalCount(), 2u);
  QCOMPARE(layout.rawOffset(ModuleId{0}), 0u);

  QCOMPARE(layout.calculatedSignalCount(), 3u);

  auto calc0 = layout.location(SignalId{2});
  QCOMPARE(calc0.area, SignalMemoryArea::Calculated);
  QCOMPARE(calc0.index, 0u);

  auto calc1 = layout.location(SignalId{3});
  QCOMPARE(calc1.area, SignalMemoryArea::Calculated);
  QCOMPARE(calc1.index, 1u);

  auto calc2 = layout.location(SignalId{4});
  QCOMPARE(calc2.area, SignalMemoryArea::Calculated);
  QCOMPARE(calc2.index, 2u);
}

void tst_datasource::test_datasource_layout_two_build()
{
  using namespace qds;
  SystemConfiguration sc = createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(sc);

  QCOMPARE(layout.rawSignalCount(), 2u);
  QCOMPARE(layout.rawOffset(ModuleId{0}), 0u);
  QCOMPARE(layout.calculatedSignalCount(), 3u);

  layout.build(sc);

  QCOMPARE(layout.rawSignalCount(), 2u);
  QCOMPARE(layout.rawOffset(ModuleId{0}), 0u);
  QCOMPARE(layout.calculatedSignalCount(), 3u);
}

void tst_datasource::test_datasource_manager()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  BufferManager buffers;
  buffers.initialize(layout);

  auto &memory = buffers.beginWrite();
  auto &raw = memory.raw();

  DataSourceFactory factory;
  QVERIFY(factory.registerType(
    ModuleType::Fake,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FakeDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;
  QVERIFY(manager.initialize(
    cfg,
    layout,
    factory));

  QVERIFY(manager.acquire(memory.raw()));

  std::array<double, 7> array;
  raw.snapshot(array);

  QCOMPARE(array[0], 1);
  QCOMPARE(array[1], 2);
  QCOMPARE(array[2], 1);
  QCOMPARE(array[3], 2);
  QCOMPARE(array[4], 3);
  QCOMPARE(array[5], 1);
  QCOMPARE(array[6], 2);
}

void tst_datasource::test_datasource_fail_datasource()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Fail_ModuleType();

  SignalMemoryLayout layout;
  layout.build(cfg);

  BufferManager buffers;
  buffers.initialize(layout);

  auto &memory = buffers.beginWrite();
  auto &raw = memory.raw();

  DataSourceFactory factory;
  QVERIFY(factory.registerType(
    ModuleType::Fake,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FakeDataSource>(
        cfg.settings);
    }));

  QVERIFY(factory.registerType(
    ModuleType::Fail,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FailingDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;
  QVERIFY(manager.initialize(
    cfg,
    layout,
    factory));

  QVERIFY(!manager.acquire(memory.raw()));

  std::array<double, 6> array;
  raw.snapshot(array);

  QCOMPARE(array[0], 1);
  QCOMPARE(array[1], 2);
  QCOMPARE(array[2], 0);
  QCOMPARE(array[3], 0);
  QCOMPARE(array[4], 0);
  QCOMPARE(array[5], 0);
}

void tst_datasource::test_datasource_absent_datasource()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  BufferManager buffers;
  buffers.initialize(layout);

  auto &memory = buffers.beginWrite();

  DataSourceFactory factory;
  // Нужен FakeDataSource, его нет
  QVERIFY(factory.registerType(
    ModuleType::Fail,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FailingDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;
  QVERIFY(!manager.initialize(
    cfg,
    layout,
    factory));

  QCOMPARE(manager.size(), 0);

  QCOMPARE(layout.rawOffset(ModuleId{0}), 0);
  QCOMPARE(layout.rawOffset(ModuleId{1}), 2);
  QCOMPARE(layout.rawOffset(ModuleId{2}), 5);
}

void tst_datasource::test_datasource_missing_datasource()
{
  using namespace qds;
  SystemConfiguration cfg =
    createTestConfig_Fail_DataSource();

  SignalMemoryLayout layout;
  layout.build(cfg);

  DataSourceFactory factory;
  QVERIFY(factory.registerType(
    ModuleType::Fake,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FakeDataSource>(
        cfg.settings);
    }));

  QVERIFY(factory.registerType(
    ModuleType::Fail,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FailingDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;
  QVERIFY(!manager.initialize(
    cfg,
    layout,
    factory));

  QCOMPARE(manager.size(), 0);
}

void tst_datasource::test_datasource_repeat_initialize()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  BufferManager buffers;
  buffers.initialize(layout);

  auto &memory = buffers.beginWrite();
  auto &raw = memory.raw();

  DataSourceFactory factory;
  QVERIFY(factory.registerType(
    ModuleType::Fake,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FakeDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;
  QVERIFY(manager.initialize(
    cfg,
    layout,
    factory));

  QCOMPARE(manager.size(), 3);

  auto size = manager.size();

  QVERIFY(manager.acquire(raw));

  std::array<double, 7> values;
  raw.snapshot(values);

  QCOMPARE(values[0], 1.0);
  QCOMPARE(values[1], 2.0);
  QCOMPARE(values[2], 1.0);
  QCOMPARE(values[3], 2.0);
  QCOMPARE(values[4], 3.0);
  QCOMPARE(values[5], 1.0);
  QCOMPARE(values[6], 2.0);

  // повторная инициализация допустима, список источников создается заново

  QVERIFY(manager.initialize(
    cfg,
    layout,
    factory));

  QCOMPARE(manager.size(), size);
}

void tst_datasource::test_datasource_acquire_repeat()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  BufferManager buffers;
  buffers.initialize(layout);

  auto& raw =
    buffers.beginWrite().raw();

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Fake,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FakeDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;

  QVERIFY(manager.initialize(
    cfg,
    layout,
    factory));

  std::array<double, 7> values;

  QVERIFY(manager.acquire(raw));

  raw.snapshot(values);

  QCOMPARE(values[0], 1.0);
  QCOMPARE(values[1], 2.0);
  QCOMPARE(values[2], 1.0);
  QCOMPARE(values[3], 2.0);
  QCOMPARE(values[4], 3.0);
  QCOMPARE(values[5], 1.0);
  QCOMPARE(values[6], 2.0);

  QVERIFY(manager.acquire(raw));

  raw.snapshot(values);

  QCOMPARE(values[0], 3.0);
  QCOMPARE(values[1], 4.0);
  QCOMPARE(values[2], 4.0);
  QCOMPARE(values[3], 5.0);
  QCOMPARE(values[4], 6.0);
  QCOMPARE(values[5], 3.0);
  QCOMPARE(values[6], 4.0);
}

void tst_datasource::test_datasource_empty_config()
{
  using namespace qds;

  SystemConfiguration cfg;

  SignalMemoryLayout layout;
  layout.build(cfg);

  DataSourceFactory factory;

  DataSourceManager manager;

  QVERIFY(manager.initialize(
    cfg,
    layout,
    factory));

  QCOMPARE(manager.size(), 0);
}

void tst_datasource::test_dataSourceManager_successInit()
{
  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  BufferManager buffers;
  buffers.initialize(layout);

  DataSourceFactory factory;
  QVERIFY(factory.registerType(
    ModuleType::Fake,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FakeDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;

  auto& raw =
    buffers.beginWrite().raw();

  QVERIFY(manager.initialize(
    cfg,
    layout,
    factory));

  QCOMPARE(manager.size(), cfg.modules().size());

  QCOMPARE(layout.rawOffset(ModuleId{0}), 0);
  QCOMPARE(layout.rawOffset(ModuleId{1}), 2);
  QCOMPARE(layout.rawOffset(ModuleId{2}), 5);
}

void tst_datasource::test_datasource_fail_repeat()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  DataSourceFactory factory;

  QVERIFY(factory.registerType(
    ModuleType::Fake,
    [](const ModuleConfiguration& cfg)
    {
      return std::make_unique<FakeDataSource>(
        cfg.settings);
    }));

  DataSourceManager manager;

  QVERIFY(manager.initialize(
    cfg,
    layout,
    factory));

  QCOMPARE(manager.size(), 3);

  // Повторная неуспешная инициализация:
  // старые источники должны быть удалены.
  DataSourceFactory factory2;

  QVERIFY(!manager.initialize(
    cfg,
    layout,
    factory2));

  QCOMPARE(manager.size(), 0);
}