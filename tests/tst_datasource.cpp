#include "tst_datasource.h"
#include "generatordatasource.h"
#include "periodicdatasourcerunner.h"
#include "testsrv.h"
#include <qtestcase.h>
#include <qtestsupport_core.h>

tst_datasource::tst_datasource() { }
tst_datasource::~tst_datasource() = default;

void tst_datasource::test_generatorDataSource_once()
{
  using namespace qds;
  // создаем конфигурацию
  constexpr TagId tags[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags, std::size(tags));

  TestSrv srv(cfg);

  GeneratorDataSource source(srv.storage, cfg);
  QVERIFY(source.start());
  QVERIFY(source.isRunning());
  QVERIFY(!source.start());
  QVERIFY(source.isRunning());

  source.stop();
  QVERIFY(!source.isRunning());

  uint64_t t(1234567);
  QVERIFY(source.generateOnce(t));

  Sample sample;
  QVERIFY(srv.storage.read(tags[0], sample));
  QCOMPARE(sample.value, 0.f);
  QVERIFY(srv.storage.read(tags[1], sample));
  QCOMPARE(sample.value, 1.f);
  QCOMPARE(srv.storage.timestamp(tags[0]), t);
  QCOMPARE(srv.storage.timestamp(tags[1]), t);

  uint64_t t2 = 7777777;

  QVERIFY(source.generateOnce(t2));

  QVERIFY(srv.storage.read(tags[0], sample));
  QCOMPARE(sample.value, 2.f);

  QVERIFY(srv.storage.read(tags[1], sample));
  QCOMPARE(sample.value, 3.f);

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

  TestSrv srv(cfg);

  GeneratorDataSource source(srv.storage, cfg);

  // ---------- First generation ----------
  uint64_t t(1234567);
  QVERIFY(source.generateOnce(t));

  Sample sample;

  // проверки
  const auto& tags = cfg.moduleTags({0}); // 2 тега
  QVERIFY(srv.storage.read(tags[0], sample));
  QCOMPARE(sample.value, 0.f);
  QVERIFY(srv.storage.read(tags[1], sample));
  QCOMPARE(sample.value, 1.f);

  QCOMPARE(srv.storage.timestamp(tags[0]), t);
  QCOMPARE(srv.storage.timestamp(tags[1]), t);

  const auto& tags2 = cfg.moduleTags({1}); // 3 тега
  QVERIFY(srv.storage.read(tags2[0], sample));
  QCOMPARE(sample.value, 0.f);
  QVERIFY(srv.storage.read(tags2[1], sample));
  QCOMPARE(sample.value, 1.f);
  QVERIFY(srv.storage.read(tags2[2], sample));
  QCOMPARE(sample.value, 2.f);

  QCOMPARE(srv.storage.timestamp(tags2[0]), t);
  QCOMPARE(srv.storage.timestamp(tags2[1]), t);
  QCOMPARE(srv.storage.timestamp(tags2[2]), t);

  QCOMPARE(srv.storage.moduleTimestamp(cfg.modules()[0].id), t);
  QCOMPARE(srv.storage.moduleTimestamp(cfg.modules()[1].id), t);

  // ---------- Second generation ----------
  uint64_t t2 = 9999999;
  QVERIFY(source.generateOnce(t2));

  // проверки
  QVERIFY(srv.storage.read(tags[0], sample));
  QCOMPARE(sample.value, 2.f);
  QVERIFY(srv.storage.read(tags[1], sample));
  QCOMPARE(sample.value, 3.f);

  QCOMPARE(srv.storage.timestamp(tags[0]), t2);
  QCOMPARE(srv.storage.timestamp(tags[1]), t2);

  QVERIFY(srv.storage.read(tags2[0], sample));
  QCOMPARE(sample.value, 3.f);
  QVERIFY(srv.storage.read(tags2[1], sample));
  QCOMPARE(sample.value, 4.f);
  QVERIFY(srv.storage.read(tags2[2], sample));
  QCOMPARE(sample.value, 5.f);

  QCOMPARE(srv.storage.timestamp(tags2[0]), t2);
  QCOMPARE(srv.storage.timestamp(tags2[1]), t2);
  QCOMPARE(srv.storage.timestamp(tags2[2]), t2);

  QCOMPARE(srv.storage.moduleTimestamp(cfg.modules()[0].id), t2);
  QCOMPARE(srv.storage.moduleTimestamp(cfg.modules()[1].id), t2);
}

void tst_datasource::test_generatorDataSource_periodicCall()
{
  constexpr TagId tags[] { {0}, {1} };
  SystemConfiguration cfg = createTestConfig(tags, std::size(tags));

  TestSrv srv(cfg);

  GeneratorDataSource source(srv.storage, cfg);

  PeriodicDataSourceRunner runner(source);
  runner.start(10);
  QVERIFY(source.isRunning());

  QTest::qWait(1000);

  Sample beforeSample;
  QVERIFY(srv.storage.read(tags[0], beforeSample));
  QVERIFY(beforeSample.value > 0.f);
  qDebug() << "тег 0 before:" << beforeSample.value;

  QVERIFY(srv.storage.read(tags[1], beforeSample));
  QVERIFY(beforeSample.value > 0.f);
  qDebug() << "тег 1 before:" << beforeSample.value;

  QVERIFY(srv.storage.timestamp(tags[0]) > 0);
  QVERIFY(srv.storage.timestamp(tags[1]) > 0);

  const auto beforeCount = source.generationCount();

  QVERIFY(beforeCount >= 70);
  QVERIFY(beforeCount <= 90);

  uint64_t t1 = srv.storage.moduleTimestamp({0});

  qDebug() << "generationCount before" << beforeCount;

  QTest::qWait(100);

  Sample afterSample;

  QVERIFY(srv.storage.read(tags[0], afterSample));
  QVERIFY(afterSample.value > beforeSample.value);
  qDebug() << "тег 0 after:" << afterSample.value;

  QVERIFY(srv.storage.read(tags[1], afterSample));
  QVERIFY(afterSample.value > beforeSample.value);
  qDebug() << "  тег 1 after:" << afterSample.value;

  const auto afterCount = source.generationCount();

  QVERIFY(afterCount >= 80);
  QVERIFY(afterCount <= 100);

  qDebug() << "generationCount after" << afterCount;
  qDebug() << "time:" << srv.storage.moduleTimestamp({0}) - t1;

  const auto delta = afterCount - beforeCount;

  QVERIFY(delta >= 7);
  QVERIFY(delta <= 12);

  runner.stop();
  QVERIFY(!source.isRunning());

  // проверка после остановки
  Sample s1, s2;
  const auto count = source.generationCount();
  QVERIFY(srv.storage.read(tags[0], s1));

  QTest::qWait(100);

  QCOMPARE(source.generationCount(), count);
  QVERIFY(srv.storage.read(tags[0], s2));

  QCOMPARE(s1.value, s2.value);
}
