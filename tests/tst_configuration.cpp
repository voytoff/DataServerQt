#include "tst_configuration.h"
#include "systemconfiguration.h"
#include "testsrv.h"
#include <qtestcase.h>

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
      .archiveFrequency = 100,
    }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {1},
     .name = "RAW1",
     .kind = SignalKind::Raw,
     .source = {3},
     .archiveFrequency = 1000,
     }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {2},
     .name = "A",
     .kind = SignalKind::Calculated,
     .archiveFrequency = 100,
     .calibrationMode = CalibrationMode::BySignal,
     .dependencies = {{0}},
     }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {3},
     .name = "B",
     .kind = SignalKind::Calculated,
     .archiveFrequency = 100,
     .calibrationMode = CalibrationMode::BySignalType,
     .dependencies = {{1}},
     }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {4},
     .name = "C",
     .kind = SignalKind::Calculated,
     .archiveFrequency = 10,
     .formulaId = {23},
     .dependencies = {{2}, {3}},
     }));

  auto &defs = cfg.signalDefinitions();
  QCOMPARE(defs.size(), 5);

  auto def0 = cfg.findSignalDefinition(SignalId{0});
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
  QCOMPARE(def3->calibrationMode, CalibrationMode::BySignal);
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
     .archiveFrequency = 100,
     }));

  QVERIFY(!cfg.addSignalDefinition(
    {
     .id = {4},
     .name = "RAW0",
     .kind = SignalKind::Raw,
     .source = {0},
     .archiveFrequency = 100,
     }));

  QCOMPARE(cfg.signalDefinitions().size(), 1);

  QVERIFY(!cfg.addSignalDefinition(
    {
     .id = {5},
     .name = "RAW0",
     .kind = SignalKind::Raw,
     .source = {1000},
     .archiveFrequency = 100,
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
     .archiveFrequency = 100,
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
     .archiveFrequency = 100,
     }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {4},
     .name = "RAW1",
     .kind = SignalKind::Raw,
     .source = {2},
     .archiveFrequency = 1000,
     }));

  QVERIFY(cfg.addSignalDefinition(
    {
     .id = {2},
     .name = "A",
     .kind = SignalKind::Calculated,
     .archiveFrequency = 100,
     .calibrationMode = CalibrationMode::BySignalType,
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