#include "tst_configuration.h"
#include "applicationsettingsloader.h"
#include "calibration.h"
#include "db/configurationrepository.h"
#include "db/sensorrepository.h"
#include "qds/db.h"
#include "systemconfiguration.h"
#include "testsrv.h"
#include <qtestcase.h>
#include <stdint.h>

tst_configuration::tst_configuration() { }
tst_configuration::~tst_configuration() = default;

void tst_configuration::test_configuration_crates()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addCrate({0});
  cfg.addCrate({7});

  QCOMPARE(cfg.crates().size(), 2);
}

void tst_configuration::test_configuration_modules()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {0}, .crate = {0}, .type = ModuleType::Failing});
  cfg.addModule({.id = {100}, .crate = {7}, .type = ModuleType::Fake});
  cfg.addModule({.id = {10}, .crate = {3}, .type = ModuleType::Test});

  QCOMPARE(cfg.modules().size(), 3);
}

void tst_configuration::test_configuration_tags()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {0}, .crate = {0}});
  QVERIFY(cfg.addTag({ .tag = {1}, .module = {0}, .channel = {0}}));

  cfg.addModule({.id = {100}, .crate = {7}, .type = ModuleType::Fake});
  QVERIFY(cfg.addTag({ .tag = {11}, .module = {100}, .channel = {0}}));
  QVERIFY(cfg.addTag({ .tag = {2}, .module = {100}, .channel = {1}}));
  QVERIFY(cfg.addTag({ .tag = {3}, .module = {100}, .channel = {1}}));

  QCOMPARE(cfg.tags().size(), 4);
  QCOMPARE(cfg.moduleTags({0}).size(), 1);
  QCOMPARE(cfg.moduleTags({100}).size(), 3);

  QVERIFY(cfg.containsTag({1}));
  QVERIFY(cfg.containsTag({2}));
  QVERIFY(cfg.containsTag({3}));
  QVERIFY(cfg.containsTag({11}));

  QVERIFY(!cfg.containsTag({12}));

  const auto *tagInfo = cfg.findTag({3});
  QVERIFY(tagInfo != nullptr);
  QCOMPARE(tagInfo->module, ModuleId{100});

  tagInfo = cfg.findTag({4});
  QVERIFY(tagInfo == nullptr);
}

void tst_configuration::test_configuration_parse_module_ids()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {0}, .crate = {0}, .type = ModuleType::Failing});
  QVERIFY(cfg.addTag({ .tag = {1}, .module = {0}, .channel = {0}}));

  cfg.addModule({.id = {100}, .crate = {7}, .type = ModuleType::Fake});
  QVERIFY(cfg.addTag({ .tag = {11}, .module = {100}, .channel = {0}}));
  QVERIFY(cfg.addTag({ .tag = {2}, .module = {100}, .channel = {1}}));

  cfg.addModule({.id = {10}, .crate = {3}, .type = ModuleType::Test});
  QVERIFY(cfg.addTag({ .tag = {3}, .module = {10}, .channel = {0}}));

  QCOMPARE(cfg.modules()[0].crate.value, 0);
  QCOMPARE(cfg.modules()[0].type, ModuleType::Failing);

  auto &tags0 = cfg.moduleTags({0});
  QCOMPARE(tags0.size(), 1);
  QCOMPARE(tags0[0].value, 1);

  QCOMPARE(cfg.modules()[1].crate.value, 7);
  QCOMPARE(cfg.modules()[1].type, ModuleType::Fake);

  auto &tags1 = cfg.moduleTags({100});
  QCOMPARE(tags1.size(), 2);
  QCOMPARE(tags1[0].value, 11);
  QCOMPARE(tags1[1].value, 2);

  QCOMPARE(cfg.modules()[2].crate.value, 3);
  QCOMPARE(cfg.modules()[2].type, ModuleType::Test);

  auto &tags2 = cfg.moduleTags({10});
  QCOMPARE(tags2.size(), 1);
  QCOMPARE(tags2[0].value, 3);
}

void tst_configuration::test_configuration_moduleTags()
{
  using namespace qds;
  SystemConfiguration cfg;

  cfg.addModule({.id = {0}, .crate = {0}});
  QVERIFY(cfg.addTag({ .tag = {1}, .module = {0}, .channel = {0}}));

  cfg.addModule({.id = {100}, .crate = {7}, .type = ModuleType::Fake});
  QVERIFY(cfg.addTag({ .tag = {11}, .module = {100}, .channel = {0}}));
  QVERIFY(cfg.addTag({ .tag = {2}, .module = {100}, .channel = {1}}));
  QVERIFY(cfg.addTag({ .tag = {3}, .module = {100}, .channel = {2}}));

  const auto &tags0 = cfg.moduleTags({0});
  QCOMPARE(tags0.size(), 1);

  const auto &tags100 = cfg.moduleTags({100});
  QCOMPARE(tags100.size(), 3);

  QCOMPARE(tags100[0].value, 11);
  QCOMPARE(tags100[1].value, 2);
  QCOMPARE(tags100[2].value, 3);
}

void tst_configuration::test_configuration_moduleChannelCount()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {0}, .crate = {0}});
  QVERIFY(cfg.addTag({ .tag = {1}, .module = {0}, .channel = {0}}));
  QVERIFY(cfg.addTag({ .tag = {3}, .module = {0}, .channel = {1}}));

  cfg.addModule({.id = {100}, .crate = {7}});
  QVERIFY(cfg.addTag({ .tag = {11}, .module = {100}, .channel = {0}}));
  QVERIFY(cfg.addTag({ .tag = {2}, .module = {100}, .channel = {1}}));
  QVERIFY(cfg.addTag({ .tag = {7}, .module = {100}, .channel = {2}}));
  QVERIFY(cfg.addTag({ .tag = {202}, .module = {100}, .channel = {3}}));

  QCOMPARE(cfg.moduleChannelCount({0}), 2);
  QCOMPARE(cfg.moduleChannelCount({100}), 4);
}

void tst_configuration::test_configuration_signalDefinitions()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {7}, .crate = {0}});
  QVERIFY(cfg.addTag({ .tag = {1}, .module = {7}, .channel = {0}}));
  QVERIFY(cfg.addTag({ .tag = {3}, .module = {7}, .channel = {1}}));

  QVERIFY(cfg.addSignalDefinition(
    {
      .id = {0},
      .name = "RAW0",
      .kind = SignalKind::Raw,
      .source = {1},
      .archiveRate = ArchiveRate::Hz100,
    }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {1},
     .name = "RAW1",
     .kind = SignalKind::Raw,
     .source = {3},
     .archiveRate = ArchiveRate::Hz1000,
     }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {2},
     .name = "A",
     .kind = SignalKind::Calculated,
     .archiveRate = ArchiveRate::Hz100,
     .dependencies = {{0}},
     }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {3},
     .name = "B",
     .kind = SignalKind::Calculated,
     .archiveRate = ArchiveRate::Hz100,
     .dependencies = {{1}},
     }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {4},
     .name = "C",
     .kind = SignalKind::Calculated,
     .archiveRate = ArchiveRate::Hz10,
     .formulaId = {23},
     .dependencies = {{2}, {3}},
     }));

  auto &defs = cfg.signalDefinitions();
  QCOMPARE(defs.size(), 5);

  auto def0 = cfg.findSignalDefinition(SignalId{0});
  QVERIFY(def0 != nullptr);
  QCOMPARE(def0->id.value, 0);
  QCOMPARE(def0->kind, SignalKind::Raw);
  QCOMPARE(def0->archiveRate, ArchiveRate::Hz100);
  QCOMPARE(def0->source.tag, {1});

  auto def3 = cfg.findSignalDefinition({3});
  QVERIFY(def3 != nullptr);
  QCOMPARE(def3->id.value, 3);
  QCOMPARE(def3->kind, SignalKind::Calculated);
  QCOMPARE(def3->archiveRate, ArchiveRate::Hz100);
  QCOMPARE(def3->dependencies.size(), 1);
  QCOMPARE(def3->dependencies[0].value, 1);

  auto def4 = cfg.findSignalDefinition({4});
  QVERIFY(def4 != nullptr);
  QCOMPARE(def4->id.value, 4);
  QCOMPARE(def4->kind, SignalKind::Calculated);
  QCOMPARE(def4->archiveRate, ArchiveRate::Hz10);
  QCOMPARE(def4->formulaId, {23});
  QCOMPARE(def4->dependencies.size(), 2);
  QCOMPARE(def4->dependencies[0].value, 2);
  QCOMPARE(def4->dependencies[1].value, 3);

  QVERIFY(cfg.findSignalDefinition({1000}) == nullptr);
  QVERIFY(!cfg.containsSignalDefinition({1000}));
}

void tst_configuration::test_configuration_empty()
{
  using namespace qds;

  SystemConfiguration cfg;

  QCOMPARE(cfg.crates().size(), 0);
  QCOMPARE(cfg.modules().size(), 0);
  QCOMPARE(cfg.tags().size(), 0);
  QCOMPARE(cfg.signalDefinitions().size(), 0);

  QVERIFY(!cfg.containsTag({0}));
  QVERIFY(cfg.findTag({0}) == nullptr);

  QVERIFY(!cfg.containsSignalDefinition({0}));
  QVERIFY(cfg.findSignalDefinition(SignalId{0}) == nullptr);
}

void tst_configuration::test_configuration_sparseTagIds()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {0}, .crate = {0}});

  QVERIFY(cfg.addTag({
    .tag = {1000},
    .module = {0},
    .channel = {0}
  }));

  QVERIFY(!cfg.containsTag({999}));
  QVERIFY(cfg.containsTag({1000}));

  QVERIFY(cfg.findTag({999}) == nullptr);

  const auto* tag = cfg.findTag({1000});
  QVERIFY(tag != nullptr);
  QCOMPARE(tag->tag, TagId{1000});
}

void tst_configuration::test_configuration_duplicateTagIds()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {0}, .crate = {0}});

  QVERIFY(cfg.addTag({
    .tag = {0},
    .module = {0},
    .channel = {0}
  }));

  QVERIFY(!cfg.addTag({
    .tag = {0},
    .module = {0},
    .channel = {1}
  }));

  QCOMPARE(cfg.tags().size(), 1);
  QCOMPARE(cfg.moduleTags({0}).size(), 1);
}

void tst_configuration::test_configuration_duplicateSignalDefinitions()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {0}, .crate = {0}});

  QVERIFY(cfg.addTag({
    .tag = {0},
    .module = {0},
    .channel = {0}
  }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {4},
     .name = "RAW0",
     .kind = SignalKind::Raw,
     .source = {0},
     .archiveRate = ArchiveRate::Hz100,
     }));

  QVERIFY(!cfg.addSignalDefinition(
    {
     .id = {4},
     .name = "RAW0",
     .kind = SignalKind::Raw,
     .source = {0},
     .archiveRate = ArchiveRate::Hz100,
     }));

  QCOMPARE(cfg.signalDefinitions().size(), 1);

  QVERIFY(!cfg.addSignalDefinition(
    {
     .id = {5},
     .name = "RAW0",
     .kind = SignalKind::Raw,
     .source = {1000},
     .archiveRate = ArchiveRate::Hz100,
     }));

  QCOMPARE(cfg.signalDefinitions().size(), 1);

  QVERIFY(!cfg.containsSignalDefinition({5}));
  QVERIFY(cfg.findSignalDefinition({5}) == nullptr);
}

void tst_configuration::test_configuration_failTagModule()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {1}, .crate = {0}});

  QVERIFY(!cfg.addTag({
    .tag = {100},
    .module = {77},
    .channel = {0}
  }));

  QCOMPARE(cfg.tags().size(), 0);
  QVERIFY(!cfg.containsTag({100}));
}

void tst_configuration::test_configuration_sparseSignalDefinitionIds()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {0}, .crate = {0}});

  QVERIFY(cfg.addTag({
    .tag = {0},
    .module = {0},
    .channel = {0}
  }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {1000},
     .name = "RAW0",
     .kind = SignalKind::Raw,
     .source = {0},
     .archiveRate = ArchiveRate::Hz100,
     }));

  QVERIFY(!cfg.containsSignalDefinition({999}));
  QVERIFY(cfg.containsSignalDefinition({1000}));

  QVERIFY(cfg.findSignalDefinition({999}) == nullptr);

  const auto* def = cfg.findSignalDefinition({1000});
  QVERIFY(def != nullptr);
  QCOMPARE(def->id, SignalId{1000});
}

void tst_configuration::test_configuration_orderIds()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {0}, .crate = {0}});
  QVERIFY(cfg.addTag({ .tag = {1}, .module = {0}, .channel = {0}}));
  QVERIFY(cfg.addTag({ .tag = {3}, .module = {0}, .channel = {1}}));

  cfg.addModule({.id = {1}, .crate = {7}});
  QVERIFY(cfg.addTag({ .tag = {11}, .module = {1}, .channel = {0}}));
  QVERIFY(cfg.addTag({ .tag = {2}, .module = {1}, .channel = {1}}));
  QVERIFY(cfg.addTag({ .tag = {202}, .module = {1}, .channel = {3}}));
  QVERIFY(cfg.addTag({ .tag = {7}, .module = {1}, .channel = {2}}));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {0},
     .name = "RAW0",
     .kind = SignalKind::Raw,
     .source = {1},
     .archiveRate = ArchiveRate::Hz100,
     }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {4},
     .name = "RAW1",
     .kind = SignalKind::Raw,
     .source = {2},
     .archiveRate = ArchiveRate::Hz1000,
     }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {2},
     .name = "A",
     .kind = SignalKind::Calculated,
     .archiveRate = ArchiveRate::Hz100,
    .dependencies = {{4}, {0}},
     }));

  const auto &tags = cfg.tags();
  QCOMPARE(tags.size(), 6);

  QCOMPARE(tags[0].tag, TagId{1});
  QCOMPARE(tags[1].tag, TagId{3});
  QCOMPARE(tags[2].tag, TagId{11});
  QCOMPARE(tags[3].tag, TagId{2});
  QCOMPARE(tags[4].tag, TagId{202});
  QCOMPARE(tags[5].tag, TagId{7});

  const auto &moduleTags = cfg.moduleTags({1});
  QCOMPARE(moduleTags.size(), 4);

  QCOMPARE(moduleTags[0], TagId{11});
  QCOMPARE(moduleTags[1], TagId{2});
  QCOMPARE(moduleTags[2], TagId{202});
  QCOMPARE(moduleTags[3], TagId{7});


  const auto &defs = cfg.signalDefinitions();
  QCOMPARE(defs.size(), 3);

  QCOMPARE(defs[0].id, SignalId{0});
  QCOMPARE(defs[1].id, SignalId{4});
  QCOMPARE(defs[2].id, SignalId{2});
}

void tst_configuration::test_configuration_findSignalDefinition()
{
  using namespace qds;

  SystemConfiguration cfg = createTestConfig_Copy_Add();

  const auto* raw0 = cfg.findSignalDefinition("Raw0");
  QVERIFY(raw0 != nullptr);
  QCOMPARE(raw0->kind, SignalKind::Raw);
  QCOMPARE(raw0->id, SignalId{0});

  const auto *raw1 = cfg.findSignalDefinition("Raw1");
  QVERIFY(raw1 != nullptr);
  QCOMPARE(raw1->kind, SignalKind::Raw);
  QCOMPARE(raw1->id, SignalId{1});

  const auto *a = cfg.findSignalDefinition("A");
  QVERIFY(a != nullptr);
  QCOMPARE(a->kind, SignalKind::Calculated);
  QCOMPARE(a->id, SignalId{2});

  const auto *b = cfg.findSignalDefinition("B");
  QVERIFY(b != nullptr);
  QCOMPARE(b->kind, SignalKind::Calculated);
  QCOMPARE(b->id, SignalId{3});

  const auto *c = cfg.findSignalDefinition("C");
  QVERIFY(c != nullptr);
  QCOMPARE(c->kind, SignalKind::Calculated);
  QCOMPARE(c->id, SignalId{4});

  QVERIFY(cfg.findSignalDefinition("NONE") == nullptr);
}

void tst_configuration::test_configuration_polynomial()
{
  Calibration calibration;

  calibration.kind = CalibrationKind::Polynomial;
  calibration.coefficients = {2.0, 3.0, 4.0};

  double result = 0.0;

  QVERIFY(calibration.apply(2.0, result));
  QCOMPARE(result, 24.0);
}

void tst_configuration::test_sensorRepository()
{
  SensorRepository repository;

  QVERIFY(repository.add(
    Sensor{
      .id = SensorId{1},
      .sensorTypeId = SensorTypeId{1},
      .name = "POWER",
      .serial = "SERIAL01",
      .calibrationMode = SensorCalibrationMode::Own,
      .calibrationKind = CalibrationKind::Table,
    }));

  QCOMPARE(repository.sensorCount(), 1);

  QVERIFY(repository.add(
    SensorType{
      .id = SensorTypeId{1},
      .name = "SENST01",
      .calibrationKind = CalibrationKind::Polynomial,
  }));

  QCOMPARE(repository.sensorTypeCount(), 1);

  QVERIFY(repository.find(SensorId{1}));

  QVERIFY(!repository.add(
    Sensor{
      .id = SensorId{1},
      .name = "DUPLICATE"
    }));

  repository.clear();
  QCOMPARE(repository.sensorCount(), 0);
  QCOMPARE(repository.sensorTypeCount(), 0);
}

void tst_configuration::test_loadSensors()
{
  using namespace qds;

  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository database(db);
  SensorRepository sensors;

  QVERIFY(database.loadSensors(sensors));

  QVERIFY(sensors.sensorTypeCount() > 0);
  QVERIFY(sensors.sensorCount() > 0);
}

void tst_configuration::test_createTableCalibration()
{
  using namespace qds;

  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repository(db);
  SensorRepository sr;

  QVERIFY(repository.loadSensors(sr));

  const auto sensor = sr.sensors()[0];

  Calibration calibration;

  // Указываем существующий датчик в тестовой БД.
  calibration.sensorId = SensorId{sensor.id};
  calibration.kind = CalibrationKind::Table;
  calibration.description = "Created by Qt Test";

  calibration.points = {
    {0, 0.0, 0.0},
    {1, 10.0, 20.0},
    {2, 20.0, 50.0}
  };

  // Создание в MariaDB.
  QVERIFY2(
    repository.createCalibration(calibration),
    qPrintable(repository.lastError().text()));

  QVERIFY(calibration.id.value != 0);

  qDebug() << "Created calibration ID:"
           << calibration.id.value;

  // Проверяем, что сегменты рассчитаны.
  QCOMPARE(calibration.segments.size(), std::size_t{2});

  // Проверяем вычисление.
  double result = 0.0;

  QVERIFY(calibration.apply(5.0, result));
  QCOMPARE(result, 10.0);

  QVERIFY(calibration.apply(15.0, result));
  QCOMPARE(result, 35.0);

  // Проверяем экстраполяцию.
  QVERIFY(calibration.apply(-5.0, result));
  QCOMPARE(result, -10.0);

  QVERIFY(calibration.apply(30.0, result));
  QCOMPARE(result, 80.0);

  // Удаляем созданную тарировку.
  QVERIFY2(
    repository.removeCalibration(calibration.id),
    qPrintable(repository.lastError().text()));

  QSqlQuery query(db);

  query.prepare(R"(
SELECT COUNT(*)
FROM calibration
WHERE id = :id;)");

  query.bindValue(":id", calibration.id.value);

  QVERIFY(query.exec());
  QVERIFY(query.next());
  QCOMPARE(query.value(0).toInt(), 0);
}

void tst_configuration::test_updateTableCalibration()
{
  using namespace qds;

  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repository(db);
  SensorRepository sr;

  QVERIFY(repository.loadSensors(sr));
  QVERIFY(!sr.sensors().empty());

  const auto sensor = sr.sensors()[0];

  Calibration calibration;
  calibration.sensorId = SensorId{sensor.id};
  calibration.kind = CalibrationKind::Table;
  calibration.description = "Before update";

  calibration.points = {
    {0, 0.0, 0.0},
    {1, 10.0, 20.0},
    {2, 20.0, 50.0}
  };

  // Создаём тарировку.
  QVERIFY2(
    repository.createCalibration(calibration),
    qPrintable(repository.lastError().text()));

  const CalibrationId id = calibration.id;

  // Изменяем название, описание и точки.
  calibration.description = "After update";

  calibration.points = {
    {0, 0.0, 0.0},
    {1, 10.0, 30.0},
    {2, 20.0, 60.0},
    {3, 30.0, 90.0}
  };

  // Обновляем тарировку в MariaDB.
  QVERIFY2(
    repository.updateCalibration(calibration),
    qPrintable(repository.lastError().text()));

  // Проверяем основные поля непосредственно в БД.
  QSqlQuery query(db);

  query.prepare(R"(
SELECT sensor_id, sensor_type_id, kind, name, description
FROM calibration
WHERE id = :id;)");

  query.bindValue(":id", id.value);

  QVERIFY(query.exec());
  QVERIFY(query.next());

  QCOMPARE(query.value(0).toUInt(), sensor.id.value);
  QVERIFY(query.value(1).isNull());
  QCOMPARE(query.value(2).toInt(),
           static_cast<int>(CalibrationKind::Table));
  QCOMPARE(query.value(3).toString(), QString("Updated calibration"));
  QCOMPARE(query.value(4).toString(), QString("After update"));

  // Проверяем, что старые точки заменены новыми.
  QSqlQuery pointsQuery(db);

  pointsQuery.prepare(R"(
SELECT `index`, x, y
FROM calibration_point
WHERE calibration_id = :id
ORDER BY `index`;)");

  pointsQuery.bindValue(":id", id.value);

  QVERIFY(pointsQuery.exec());

  const std::vector<CalibrationPoint> expected = {
    {0, 0.0, 0.0},
    {1, 10.0, 30.0},
    {2, 20.0, 60.0},
    {3, 30.0, 90.0}
  };

  std::size_t index = 0;

  while (pointsQuery.next())
  {
    QVERIFY(index < expected.size());

    const auto& point = expected[index];

    QCOMPARE(pointsQuery.value(0).toUInt(), point.index);
    QCOMPARE(pointsQuery.value(1).toDouble(), point.x);
    QCOMPARE(pointsQuery.value(2).toDouble(), point.y);

    ++index;
  }

  QCOMPARE(index, expected.size());

  // Удаляем тестовую тарировку.
  QVERIFY2(
    repository.removeCalibration(id),
    qPrintable(repository.lastError().text()));
}

void tst_configuration::test_updateTableCalibrationRollback()
{
  using namespace qds;

  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repository(db);
  SensorRepository sr;

  QVERIFY(repository.loadSensors(sr));
  QVERIFY(!sr.sensors().empty());

  const auto sensor = sr.sensors()[0];

  Calibration calibration;
  calibration.sensorId = SensorId{sensor.id};
  calibration.kind = CalibrationKind::Table;
  calibration.description = "Before update";

  calibration.points = {
    {0, 0.0, 0.0},
    {1, 10.0, 20.0},
    {2, 20.0, 50.0}
  };

  // Создаём тарировку.
  QVERIFY2(
    repository.createCalibration(calibration),
    qPrintable(repository.lastError().text()));

  const CalibrationId id = calibration.id;

  // Изменяем название, описание и точки.
  calibration.description = "After update";

  calibration.points = {
    {0, 0.0, 0.0},
    {1, 10.0, 30.0},
    {2, 20.0, 60.0},
    {3, 30.0, 90.0},
    {4, 30.0, 110.0}
  };

  // Обновляем тарировку в MariaDB.
  QVERIFY2(
    !repository.updateCalibration(calibration),
    qPrintable(repository.lastError().text()));

  // Проверяем основные поля непосредственно в БД.
  QSqlQuery query(db);

  query.prepare(R"(
SELECT sensor_id, sensor_type_id, kind, name, description
FROM calibration
WHERE id = :id;)");

  query.bindValue(":id", id.value);

  QVERIFY(query.exec());
  QVERIFY(query.next());

  QCOMPARE(query.value(0).toUInt(), sensor.id.value);
  QVERIFY(query.value(1).isNull());
  QCOMPARE(query.value(2).toInt(),
           static_cast<int>(CalibrationKind::Table));
  QCOMPARE(query.value(3).toString(), QString("Test calibration"));
  QCOMPARE(query.value(4).toString(), QString("Before update"));

  // Проверяем, что старые точки не заменились новыми.
  QSqlQuery pointsQuery(db);

  pointsQuery.prepare(R"(
SELECT `index`, x, y
FROM calibration_point
WHERE calibration_id = :id
ORDER BY `index`;)");

  pointsQuery.bindValue(":id", id.value);

  QVERIFY(pointsQuery.exec());

  const std::vector<CalibrationPoint> expected = {
    {0, 0.0, 0.0},
    {1, 10.0, 20.0},
    {2, 20.0, 50.0}
  };

  std::size_t index = 0;

  while (pointsQuery.next())
  {
    QVERIFY(index < expected.size());

    const auto& point = expected[index];

    QCOMPARE(pointsQuery.value(0).toUInt(), point.index);
    QCOMPARE(pointsQuery.value(1).toDouble(), point.x);
    QCOMPARE(pointsQuery.value(2).toDouble(), point.y);

    ++index;
  }

  QCOMPARE(index, expected.size());

  // Удаляем тестовую тарировку.
  QVERIFY2(
    repository.removeCalibration(id),
    qPrintable(repository.lastError().text()));
}

void tst_configuration::test_createPolynomialCalibration()
{
  using namespace qds;

  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repository(db);
  SensorRepository sr;

  QVERIFY(repository.loadSensors(sr));
  QVERIFY(!sr.sensors().empty());

  const auto sensor = sr.sensors()[0];

  Calibration calibration;

  calibration.sensorId = SensorId{sensor.id};
  calibration.kind = CalibrationKind::Polynomial;
  calibration.description = "Created by Qt Test";

  // y = 2 + 3*x + 4*x*x
  calibration.coefficients = {
    2.0,
    3.0,
    4.0
  };

  // Создаём полиномиальную тарировку.
  QVERIFY2(
    repository.createCalibration(calibration),
    qPrintable(repository.lastError().text()));

  const CalibrationId id = calibration.id;

  QVERIFY(id.value != 0);

  qDebug() << "Created polynomial calibration ID:"
           << id.value;

  // Проверяем вычисление полинома.
  double result = 0.0;

  QVERIFY(calibration.apply(0.0, result));
  QCOMPARE(result, 2.0);

  QVERIFY(calibration.apply(1.0, result));
  QCOMPARE(result, 9.0);

  QVERIFY(calibration.apply(2.0, result));
  QCOMPARE(result, 24.0);

  QVERIFY(calibration.apply(-1.0, result));
  QCOMPARE(result, 3.0);

  // Проверяем основные поля в MariaDB.
  QSqlQuery query(db);

  query.prepare(R"(
SELECT sensor_id, sensor_type_id, kind, name, description
FROM calibration
WHERE id = :id;)");

  query.bindValue(":id", id.value);

  QVERIFY(query.exec());
  QVERIFY(query.next());

  QCOMPARE(query.value(0).toUInt(), sensor.id.value);
  QVERIFY(query.value(1).isNull());

  QCOMPARE(
    query.value(2).toInt(),
    static_cast<int>(CalibrationKind::Polynomial));

  QCOMPARE(
    query.value(3).toString(),
    QString("Test polynomial calibration"));

  QCOMPARE(
    query.value(4).toString(),
    QString("Created by Qt Test"));

  // Проверяем сохранённые коэффициенты.
  QSqlQuery coefficientsQuery(db);

  coefficientsQuery.prepare(R"(
SELECT `index`, value
FROM calibration_coefficient
WHERE calibration_id = :id
ORDER BY `index`;)");

  coefficientsQuery.bindValue(":id", id.value);

  QVERIFY(coefficientsQuery.exec());

  const std::vector<double> expected = {
    2.0,
    3.0,
    4.0
  };

  std::size_t index = 0;

  while (coefficientsQuery.next())
  {
    QVERIFY(index < expected.size());

    QCOMPARE(
      coefficientsQuery.value(0).toUInt(),
      static_cast<uint>(index));

    QCOMPARE(
      coefficientsQuery.value(1).toDouble(),
      expected[index]);

    ++index;
  }

  QCOMPARE(index, expected.size());

  // У полиномиальной тарировки не должно быть точек.
  QSqlQuery pointsQuery(db);

  pointsQuery.prepare(R"(
SELECT COUNT(*)
FROM calibration_point
WHERE calibration_id = :id;)");

  pointsQuery.bindValue(":id", id.value);

  QVERIFY(pointsQuery.exec());
  QVERIFY(pointsQuery.next());

  QCOMPARE(pointsQuery.value(0).toInt(), 0);

  // Удаляем тестовую тарировку.
  QVERIFY2(
    repository.removeCalibration(id),
    qPrintable(repository.lastError().text()));
}

void tst_configuration::test_updatePolynomialCalibration()
{
  using namespace qds;

  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repository(db);
  SensorRepository sr;

  QVERIFY(repository.loadSensors(sr));
  QVERIFY(!sr.sensors().empty());

  const auto sensor = sr.sensors()[0];

  Calibration calibration;

  calibration.sensorId = SensorId{sensor.id};
  calibration.kind = CalibrationKind::Polynomial;
  calibration.description = "Created by Qt Test";

  // y = 2 + 3*x + 4*x*x
  calibration.coefficients = {
    2.0,
    3.0,
    4.0
  };

  // Создаём полиномиальную тарировку.
  QVERIFY2(
    repository.createCalibration(calibration),
    qPrintable(repository.lastError().text()));

  const CalibrationId id = calibration.id;

  QVERIFY(id.value != 0);

  // Изменяем название, описание и коэффициенты.
  calibration.description = "After update";

  calibration.coefficients = {
    1.0,
    2.0,
    3.0,
    4.0
  };

  // Обновляем тарировку в MariaDB.
  QVERIFY2(
    repository.updateCalibration(calibration),
    qPrintable(repository.lastError().text()));

  // y = 1 + 2*x + 3*x*x + 4*x*x*x

  double result = 0.0;

  QVERIFY(calibration.apply(0.0, result));
  QCOMPARE(result, 1.0);

  QVERIFY(calibration.apply(1.0, result));
  QCOMPARE(result, 10.0);

  QVERIFY(calibration.apply(2.0, result));
  QCOMPARE(result, 49.0);

  QVERIFY(calibration.apply(-1.0, result));
  QCOMPARE(result, -2.0);

  // Проверяем основные поля непосредственно в БД.
  QSqlQuery query(db);

  query.prepare(R"(
SELECT sensor_id, sensor_type_id, kind, name, description
FROM calibration
WHERE id = :id;)");

  query.bindValue(":id", id.value);

  QVERIFY(query.exec());
  QVERIFY(query.next());

  QCOMPARE(query.value(0).toUInt(), sensor.id.value);
  QVERIFY(query.value(1).isNull());
  QCOMPARE(query.value(2).toInt(),
           static_cast<int>(CalibrationKind::Polynomial));
  QCOMPARE(query.value(3).toString(), QString("Updated calibration"));
  QCOMPARE(query.value(4).toString(), QString("After update"));

  // Проверяем, что старый полином заменился новым.
  QSqlQuery pointsQuery(db);

  pointsQuery.prepare(R"(
SELECT `index`, value
FROM calibration_coefficient
WHERE calibration_id = :id
ORDER BY `index`;)");

  pointsQuery.bindValue(":id", id.value);

  QVERIFY(pointsQuery.exec());

  const std::vector<double> expected = {
    1.0,
    2.0,
    3.0,
    4.0
  };

  std::size_t index = 0;

  while (pointsQuery.next())
  {
    QVERIFY(index < expected.size());

    QCOMPARE(
      pointsQuery.value(0).toUInt(),
      static_cast<uint>(index));

    QCOMPARE(
      pointsQuery.value(1).toDouble(),
      expected[index]);

    ++index;
  }

  QCOMPARE(index, expected.size());

  // Удаляем тестовую тарировку.
  QVERIFY2(
    repository.removeCalibration(id),
    qPrintable(repository.lastError().text()));
}

void tst_configuration::test_updateCalibrationImmutableFields()
{
  using namespace qds;

  auto db = get_db();
  QVERIFY(db.isOpen());
  QVERIFY(db.isValid());

  ConfigurationRepository repository(db);
  SensorRepository sr;

  QVERIFY(repository.loadSensors(sr));
  QVERIFY(!sr.sensors().empty());

  const auto sensor = sr.sensors()[0];

  Calibration calibration;

  calibration.sensorId = SensorId{sensor.id};
  calibration.kind = CalibrationKind::Polynomial;
  calibration.description = "Created by Qt Test";

  // y = 2 + 3*x + 4*x*x
  calibration.coefficients = {
    2.0,
    3.0,
    4.0
  };

  // Создаём полиномиальную тарировку.
  QVERIFY2(
    repository.createCalibration(calibration),
    qPrintable(repository.lastError().text()));

  const CalibrationId id = calibration.id;

  QVERIFY(id.value != 0);

  // Изменяем название, описание и коэффициенты.
  calibration.description = "After update";

  // 1. меняем kind - вид тарировки
  calibration.kind = CalibrationKind::Table;
  calibration.points = {
    {0, 0.0, 0.0},
    {1, 10.0, 20.0}
  };
  // Пытаемся оновить тарировку в MariaDB.
  QVERIFY2(
    !repository.updateCalibration(calibration),
    qPrintable(repository.lastError().text()));


  calibration.kind = CalibrationKind::Polynomial;
  calibration.points.clear();

  // 2. меняем владельца
  // - датчик
  calibration.sensorId = SensorId{UINT32_MAX};
  // Пытаемся оновить тарировку в MariaDB.
  QVERIFY2(
    !repository.updateCalibration(calibration),
    qPrintable(repository.lastError().text()));

  calibration.sensorId = SensorId{sensor.id};

  // 3. два владельца
  calibration.sensorTypeId = SensorTypeId{UINT32_MAX};
  // Пытаемся оновить тарировку в MariaDB.
  QVERIFY2(
    !repository.updateCalibration(calibration),
    qPrintable(repository.lastError().text()));

  calibration.sensorId = SensorId{0};
  // 4. тип датчика
  // Пытаемся оновить тарировку в MariaDB.
  QVERIFY2(
    !repository.updateCalibration(calibration),
    qPrintable(repository.lastError().text()));


  // Проверяем основные поля в MariaDB.
  QSqlQuery query(db);

  query.prepare(R"(
SELECT sensor_id, sensor_type_id, kind, name, description
FROM calibration
WHERE id = :id;)");

  query.bindValue(":id", id.value);

  QVERIFY(query.exec());
  QVERIFY(query.next());

  QCOMPARE(query.value(0).toUInt(), sensor.id.value);
  QVERIFY(query.value(1).isNull());

  QCOMPARE(
    query.value(2).toInt(),
    static_cast<int>(CalibrationKind::Polynomial));

  QCOMPARE(
    query.value(3).toString(),
    QString("Test polynomial calibration"));

  QCOMPARE(
    query.value(4).toString(),
    QString("Created by Qt Test"));

  // Проверяем сохранённые коэффициенты.
  QSqlQuery coefficientsQuery(db);

  coefficientsQuery.prepare(R"(
SELECT `index`, value
FROM calibration_coefficient
WHERE calibration_id = :id
ORDER BY `index`;)");

  coefficientsQuery.bindValue(":id", id.value);

  QVERIFY(coefficientsQuery.exec());

  const std::vector<double> expected = {
    2.0,
    3.0,
    4.0
  };

  std::size_t index = 0;

  while (coefficientsQuery.next())
  {
    QVERIFY(index < expected.size());

    QCOMPARE(
      coefficientsQuery.value(0).toUInt(),
      static_cast<uint>(index));

    QCOMPARE(
      coefficientsQuery.value(1).toDouble(),
      expected[index]);

    ++index;
  }

  QCOMPARE(index, expected.size());

  // Удаляем тестовую тарировку.
  QVERIFY2(
    repository.removeCalibration(id),
    qPrintable(repository.lastError().text()));
}

#include <QTemporaryDir>
#include <QSettings>
#include <QFileInfo>

void tst_configuration::test_applicationSettingsLoader_saveConfigurationId()
{
  using namespace qds;

  QTemporaryDir directory;
  QVERIFY(directory.isValid());

  const QString fileName =
    directory.filePath("test.ini");

  const ConfigurationId id{100};

  QVERIFY(
    ApplicationSettingsLoader::saveConfigurationId(
      fileName,
      id,
      "Server/ConfigurationId"));

  QVERIFY(QFileInfo::exists(fileName));

  QSettings settings(
    fileName,
    QSettings::IniFormat);

  QCOMPARE(
    settings.value("Server/ConfigurationId").toUInt(),
    id.value);

  QCOMPARE(
    settings.status(),
    QSettings::NoError);

  QVERIFY(
    ApplicationSettingsLoader::saveConfigurationId(
      fileName,
      ConfigurationId{200},
      "Server/ConfigurationId"));

  settings.sync();

  QCOMPARE(
    settings.value("Server/ConfigurationId").toUInt(),
    200u);

  QVERIFY(
    !ApplicationSettingsLoader::saveConfigurationId(
      fileName,
      ConfigurationId{0},
      "Server/ConfigurationId"));

}