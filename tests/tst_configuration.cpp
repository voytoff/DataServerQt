#include "tst_configuration.h"
#include "systemconfiguration.h"
#include <qtestcase.h>

tst_configuration::tst_configuration() { }
tst_configuration::~tst_configuration() = default;

void tst_configuration::test_configuration_parse_module_ids()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {0}, .crate = {0}, .type = ModuleType::Fail});
  cfg.addTag({ .tag = {1}, .module = {0}, .channel = {0}});

  cfg.addModule({.id = {100}, .crate = {7}, .type = ModuleType::Fake});
  cfg.addTag({ .tag = {11}, .module = {100}, .channel = {0}});
  cfg.addTag({ .tag = {2}, .module = {100}, .channel = {1}});

  cfg.addModule({.id = {10}, .crate = {3}, .type = ModuleType::Test});
  cfg.addTag({ .tag = {3}, .module = {10}, .channel = {0}});

  QCOMPARE(cfg.modules()[0].crate.value, 0);
  QCOMPARE(cfg.modules()[0].type, ModuleType::Fail);

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

void tst_configuration::test_configuration_moduleChannelCount()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {0}, .crate = {0}});
  cfg.addTag({ .tag = {1}, .module = {0}, .channel = {0}});
  cfg.addTag({ .tag = {3}, .module = {0}, .channel = {1}});

  cfg.addModule({.id = {100}, .crate = {7}});
  cfg.addTag({ .tag = {11}, .module = {100}, .channel = {0}});
  cfg.addTag({ .tag = {2}, .module = {100}, .channel = {1}});
  cfg.addTag({ .tag = {7}, .module = {100}, .channel = {2}});
  cfg.addTag({ .tag = {202}, .module = {100}, .channel = {3}});

  QCOMPARE(cfg.moduleChannelCount({0}), 2);
  QCOMPARE(cfg.moduleChannelCount({100}), 4);
}

void tst_configuration::test_configuration_signalDefinitions()
{
  using namespace qds;

  SystemConfiguration cfg;

  cfg.addModule({.id = {7}, .crate = {0}});
  cfg.addTag({ .tag = {1}, .module = {7}, .channel = {0}});
  cfg.addTag({ .tag = {3}, .module = {7}, .channel = {1}});

  cfg.addSignalDefinition(
    {
      .id = {0},
      .name = "RAW0",
      .kind = SignalKind::Raw,
      .source = {1},
      .archiveFrequency = 100,
    });

  cfg.addSignalDefinition(
    {
     .id = {1},
     .name = "RAW1",
     .kind = SignalKind::Raw,
     .source = {3},
     .archiveFrequency = 1000,
     });

  cfg.addSignalDefinition(
    {
     .id = {2},
     .name = "A",
     .kind = SignalKind::Calculated,
     .archiveFrequency = 100,
     .calibrationId = {76},
     .dependencies = {{0}},
     });

  cfg.addSignalDefinition(
    {
     .id = {3},
     .name = "B",
     .kind = SignalKind::Calculated,
     .archiveFrequency = 100,
     .calibrationId = {67},
     .dependencies = {{1}},
     });

  cfg.addSignalDefinition(
    {
     .id = {4},
     .name = "C",
     .kind = SignalKind::Calculated,
     .archiveFrequency = 10,
     .formulaId = {23},
     .dependencies = {{2}, {3}},
     });

  auto &defs = cfg.signalDefinitions();
  QCOMPARE(defs.size(), 5);

  auto def0 = cfg.findSignalDefinition({0});
  QVERIFY(def0 != nullptr);
  QCOMPARE(def0->id.value, 0);
  QCOMPARE(def0->kind, SignalKind::Raw);
  QCOMPARE(def0->archiveFrequency, 100);
  QCOMPARE(def0->source.tag, {1});

  auto def3 = cfg.findSignalDefinition({3});
  QVERIFY(def3 != nullptr);
  QCOMPARE(def3->id.value, 3);
  QCOMPARE(def3->kind, SignalKind::Calculated);
  QCOMPARE(def3->archiveFrequency, 100);
  QCOMPARE(def3->calibrationId, {67});
  QCOMPARE(def3->dependencies.size(), 1);
  QCOMPARE(def3->dependencies[0].value, 1);

  auto def4 = cfg.findSignalDefinition({4});
  QVERIFY(def4 != nullptr);
  QCOMPARE(def4->id.value, 4);
  QCOMPARE(def4->kind, SignalKind::Calculated);
  QCOMPARE(def4->archiveFrequency, 10);
  QCOMPARE(def4->formulaId, {23});
  QCOMPARE(def4->dependencies.size(), 2);
  QCOMPARE(def4->dependencies[0].value, 2);
  QCOMPARE(def4->dependencies[1].value, 3);

  QVERIFY(cfg.findSignalDefinition({1000}) == nullptr);
  QVERIFY(!cfg.containsSignalDefinition({1000}));}
