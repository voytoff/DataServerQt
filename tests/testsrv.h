#pragma once

#include <QObject>
#include <qtestcase.h>
#include "formulafunctionabs.h"
#include "formulafunctionmax.h"
#include "formulafunctionmin.h"
#include "formulafunctionrepository.h"
#include "formulafunctionsqrt.h"
#include "isender.h"
#include "moduleinfo.h"
#include "publisher.h"
#include "subscriptionmanager.h"
#include "systemconfiguration.h"
#include "taginfo.h"
#include "udpsender.h"
#include "udpserver.h"

using namespace qds;
class TestSrv : public QObject
{
  Q_OBJECT
public:
  explicit TestSrv(const SystemConfiguration& cfg, bool udp = true,  QObject *parent = nullptr);

  SubscriptionManager manager;
  SignalMemoryLayout layout;
  SubscriptionManager subscriptions;
  UdpSender sender;
  TestPublisherSender testSender;

  std::unique_ptr<Publisher> publisher;

  PacketDispatcher dispatcher;

  UdpServer server;
};

static const SignalDefinition* findSignalDefinition(
  std::span<const SignalDefinition> array,
  const std::string &name)
{
  auto it = std::find_if(
    array.begin(),
    array.end(),
    [&name](const SignalDefinition& item)
    {
      return item.name == name;
    });

  if (it == array.end())
    return nullptr;

  return &(*it);
}

static void createSignalDefinitions(SystemConfiguration &cfg)
{
  for (const auto &m : cfg.modules())
    for (const auto &t : cfg.moduleTags({m.id}))
      cfg.addSignalDefinition({.id = {t.value}, .name = std::to_string(t.value), .kind = SignalKind::Raw, .source = {t.value}, .archiveFrequency = 100});
}

static std::unique_ptr<FormulaFunctionRepository> createFormulaFunctionRepository()
{
  FormulaFunctionRepository functions;

  if (!functions.add("sqrt", std::make_unique<FormulaFunctionSqrt>()) ||
      !functions.add("abs", std::make_unique<FormulaFunctionAbs>()) ||
      !functions.add("max", std::make_unique<FormulaFunctionMax>()) ||
      !functions.add("min", std::make_unique<FormulaFunctionMin>())
      )
    return nullptr;

  return std::make_unique<FormulaFunctionRepository>(std::move(functions));
}

static SystemConfiguration createTestConfig(const TagId* tags, int tagCount) {
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg;

  ModuleInfo m0{0};
  cfg.addModule(m0);

  for (int i = 0; i < tagCount; i++) {
    TagId t = tags[i];
    TagInfo ti;
    ti.tag = t;
    ti.module = {0};
    ti.channel = {t.value};
    cfg.addTag(ti);
  }
  createSignalDefinitions(cfg);
  return cfg;
}

/** создает конфигурацию с произвольным числом модулей */
static SystemConfiguration createTestConfig(const std::vector<std::vector<TagId>> &modules) {
  using namespace qds;
  SystemConfiguration cfg;

  for (uint32_t n = 0; n < modules.size(); n++) {
    ModuleInfo m{n};
    cfg.addModule(m);

    const auto& tags = modules[n];
    for (int i = 0; i < tags.size(); i++) {
      TagId t = tags[i];
      TagInfo ti;
      ti.tag = t;
      ti.module = m.id;
      cfg.addTag(ti);
    }
  }

  return cfg;
}

static SystemConfiguration createTestConfig00() {
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  TagInfo t1{.tag = {0}, .module = {0}, .channel = {0}};
  cfg.addTag(t1);

  TagInfo t2{.tag = {1}, .module = {0}, .channel = {0}};
  cfg.addTag(t2);

  SignalDefinition sd1 {.id = {0}, .name = "U000", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd1);

  SignalDefinition sd2 {.id = {1}, .name = "D10", .kind = SignalKind::Calculated, .source = {0}, .archiveFrequency = 10, .calibrationMode = CalibrationMode::BySignal};
  cfg.addSignalDefinition(sd2);

  SignalDefinition sd3 {.id = {2}, .name = "P100", .kind = SignalKind::Calculated, .source = {0}, .archiveFrequency = 100, .formulaId = {15}};
  cfg.addSignalDefinition(sd3);

  SignalDefinition sd4 {.id = {3}, .name = "U001", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 10};
  cfg.addSignalDefinition(sd4);

  return cfg;
}

static SystemConfiguration createTestConfig01()
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = {0}, .channel = {1}});

  SignalDefinition sd0 {.id = {0}, .name = "U000", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd0);

  SignalDefinition sd1 {.id = {1}, .name = "U001", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 10};
  cfg.addSignalDefinition(sd1);

  SignalDefinition sd2 {.id = {2}, .name = "D10", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {3}, .dependencies = {{0}, {1}}};
  cfg.addSignalDefinition(sd2);

  SignalDefinition sd3 {.id = {3}, .name = "P100", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {15}, .dependencies = {{1}, {2}}};
  cfg.addSignalDefinition(sd3);

  return cfg;
}

/**
Raw0 → A

Raw1 → B

A B → C
 */
static SystemConfiguration createTestConfig02()
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = {0}, .channel = {1}});

  SignalDefinition sd0 {.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd0);

  SignalDefinition sd1 {.id = {1}, .name = "Raw1", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 10};
  cfg.addSignalDefinition(sd1);

  SignalDefinition sd2 {.id = {2}, .name = "A", .kind = SignalKind::Calculated, .archiveFrequency = 100, .dependencies = {{0}}};
  cfg.addSignalDefinition(sd2);

  SignalDefinition sd3 {.id = {3}, .name = "B", .kind = SignalKind::Calculated, .archiveFrequency = 10, .dependencies = {{1}}};
  cfg.addSignalDefinition(sd3);

  SignalDefinition sd4 {.id = {4}, .name = "C", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {5}, .dependencies = {{2}, {3}}};
  cfg.addSignalDefinition(sd4);

  return cfg;
}

static SystemConfiguration createTestConfig03()
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = {0}, .channel = {1}});

  SignalDefinition sd0 {.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd0);

  SignalDefinition sd1 {.id = {1}, .name = "Raw1", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 10};
  cfg.addSignalDefinition(sd1);

  SignalDefinition sd2 {.id = {3}, .name = "A", .kind = SignalKind::Calculated, .archiveFrequency = 100, .dependencies = {{0}}};
  cfg.addSignalDefinition(sd2);

  SignalDefinition sd3 {.id = {5}, .name = "B", .kind = SignalKind::Calculated, .archiveFrequency = 10, .dependencies = {{1}}};
  cfg.addSignalDefinition(sd3);

  SignalDefinition sd4 {.id = {10}, .name = "C", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {5}, .dependencies = {{3}, {5}}};
  cfg.addSignalDefinition(sd4);

  return cfg;
}

/** line
Raw
 ↓
A
 ↓
B
 ↓
C
=
A
B
C
 */
static SystemConfiguration createTestConfig04()
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});

  SignalDefinition sd0 {.id = {0}, .name = "RAW", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd0);

  SignalDefinition sd1 {.id = {2}, .name = "A", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{0}}};
  cfg.addSignalDefinition(sd1);
  SignalDefinition sd2 {.id = {5}, .name = "B", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {0}, .dependencies = {{2}}};
  cfg.addSignalDefinition(sd2);
  SignalDefinition sd3 {.id = {10}, .name = "C", .kind = SignalKind::Calculated, .archiveFrequency = 1, .formulaId = {0}, .dependencies = {{5}}};
  cfg.addSignalDefinition(sd3);

  return cfg;
}

/** branching
Raw0 → A
          \
           C

Raw1 → B
 =
A
B
C
 or
B
A
C
 */
static SystemConfiguration createTestConfig05()
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = {0}, .channel = {1}});

  SignalDefinition sd0 {.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 1000};
  cfg.addSignalDefinition(sd0);
  SignalDefinition sd2 {.id = {7}, .name = "A", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{0}}};
  cfg.addSignalDefinition(sd2);
  SignalDefinition sd3 {.id = {4}, .name = "C", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {2}, .dependencies = {{7}}};
  cfg.addSignalDefinition(sd3);

  SignalDefinition sd1 {.id = {1}, .name = "Raw1", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd1);
  SignalDefinition sd4 {.id = {11}, .name = "B", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{1}}};
  cfg.addSignalDefinition(sd4);

  return cfg;
}

/** independent
Raw → A

Raw → B

Raw → C
=
 */
static SystemConfiguration createTestConfig06()
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});

  SignalDefinition sd0 {.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 1000};
  cfg.addSignalDefinition(sd0);

  SignalDefinition sd1 {.id = {101}, .name = "A", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{0}}};
  cfg.addSignalDefinition(sd1);
  SignalDefinition sd2 {.id = {70}, .name = "B", .kind = SignalKind::Calculated, .archiveFrequency = 1, .formulaId = {0}, .dependencies = {{0}}};
  cfg.addSignalDefinition(sd2);
  SignalDefinition sd3 {.id = {4}, .name = "C", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {0}, .dependencies = {{0}}};
  cfg.addSignalDefinition(sd3);

  return cfg;
}

/** cycle
A → B
B → C
C → A
 */
static SystemConfiguration createTestConfig_cycle()
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});

  SignalDefinition sd0 {.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd0);

  SignalDefinition sd1 {.id = {10}, .name = "A", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{7}}};
  cfg.addSignalDefinition(sd1);
  SignalDefinition sd2 {.id = {7}, .name = "B", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{4}}};
  cfg.addSignalDefinition(sd2);
  SignalDefinition sd3 {.id = {4}, .name = "C", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{10}}};
  cfg.addSignalDefinition(sd3);

  return cfg;
}

/** selfReference
  B → B
*/
static SystemConfiguration createTestConfig_selfReference()
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});

  SignalDefinition sd0 {.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd0);

  SignalDefinition sd1 {.id = {10}, .name = "A", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{0}}};
  cfg.addSignalDefinition(sd1);
  SignalDefinition sd2 {.id = {7}, .name = "B", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{7}}};
  cfg.addSignalDefinition(sd2);
  SignalDefinition sd3 {.id = {4}, .name = "C", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{0}}};
  cfg.addSignalDefinition(sd3);

  return cfg;
}

/** bigGraph
Raw0 → A

Raw1 → B

A,B → C

C → D

B,D → E
 */
static SystemConfiguration createTestConfig09()
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = {0}, .channel = {1}});

  SignalDefinition sd0 {.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 1000};
  cfg.addSignalDefinition(sd0);
  SignalDefinition sd1 {.id = {1}, .name = "Raw1", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd1);

  SignalDefinition sd2 {.id = {7}, .name = "A", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{0}}};
  cfg.addSignalDefinition(sd2);
  SignalDefinition sd3 {.id = {4}, .name = "B", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {0}, .dependencies = {{1}}};
  cfg.addSignalDefinition(sd3);

  SignalDefinition sd4 {.id = {11}, .name = "C", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {17}, .dependencies = {{4}, {7}}};
  cfg.addSignalDefinition(sd4);

  SignalDefinition sd5 {.id = {10}, .name = "D", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {0}, .dependencies = {{11}}};
  cfg.addSignalDefinition(sd5);

  SignalDefinition sd6 {.id = {3}, .name = "E", .kind = SignalKind::Calculated, .archiveFrequency = 1, .formulaId = {10}, .dependencies = {{4}, {10}}};
  cfg.addSignalDefinition(sd6);

  return cfg;
}

/**
Raw0 → A

Raw1 → B

C -> A + B
 =
 */
static SystemConfiguration createTestConfig_Copy_Add(ModuleType type = ModuleType::Unknown)
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{.id = {0}, .type = type};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = {0}, .channel = {1}});

  SignalDefinition sd0 {.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd0);

  SignalDefinition sd1 {.id = {1}, .name = "Raw1", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 10};
  cfg.addSignalDefinition(sd1);

  SignalDefinition sd2 {.id = {2}, .name = "A", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{0}}};
  cfg.addSignalDefinition(sd2);

  SignalDefinition sd3 {.id = {3}, .name = "B", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {0}, .dependencies = {{1}}};
  cfg.addSignalDefinition(sd3);

  SignalDefinition sd4 {.id = {4}, .name = "C", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {2}, .dependencies = {{2}, {3}}};
  cfg.addSignalDefinition(sd4);

  return cfg;
}

static SystemConfiguration createTestConfig_calculate(ModuleType type = ModuleType::Unknown)
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{.id = {0}, .type = type};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = {0}, .channel = {1}});

  cfg.addSignalDefinition({.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 1000});
  cfg.addSignalDefinition({.id = {1}, .name = "Raw1", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 100});

  cfg.addSignalDefinition({.id = {17}, .name = "A", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formula = "Raw0", .formulaId = {17}, .dependencies = {{0}}}); // Raw0

  cfg.addSignalDefinition({.id = {4}, .name = "B", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formula = "Raw1", .formulaId = {4}, .dependencies = {{1}}});  // Raw1

  cfg.addSignalDefinition({.id = {23}, .name = "C", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formula = "A + B", .formulaId = {23}, .dependencies = {{17}, {4}}}); // A & B

  cfg.setUdpPort(35000);

  return cfg;
}

static SystemConfiguration createTestConfig_Copy_Add_WithoutC()
{
  using namespace qds;

  SystemConfiguration cfg;

  ModuleInfo m{.id = {0}, .type = ModuleType::Unknown};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = {0}, .channel = {1}});

  cfg.addSignalDefinition({
    .id = {0},
    .name = "Raw0",
    .kind = SignalKind::Raw,
    .source = {0},
    .archiveFrequency = 100
  });

  cfg.addSignalDefinition({
    .id = {1},
    .name = "Raw1",
    .kind = SignalKind::Raw,
    .source = {1},
    .archiveFrequency = 10
  });

  cfg.addSignalDefinition({
    .id = {2},
    .name = "A",
    .kind = SignalKind::Calculated,
    .archiveFrequency = 100,
    .formulaId = {0},
    .dependencies = {{0}}
  });

  cfg.addSignalDefinition({
    .id = {3},
    .name = "B",
    .kind = SignalKind::Calculated,
    .archiveFrequency = 10,
    .formulaId = {0},
    .dependencies = {{1}}
  });

  return cfg;
}

static SystemConfiguration createTestConfig_Some_Modules()
{
  using namespace qds;
  SystemConfiguration cfg;

  QJsonObject jsonObj0;
  jsonObj0.insert("size", 2);

  ModuleInfo m0{.id = {0}, .type = ModuleType::Fake, .settings = jsonObj0};
  cfg.addModule(m0);

  cfg.addTag({.tag = {0}, .module = m0.id, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = m0.id, .channel = {1}});

  QJsonObject jsonObj1;
  jsonObj1.insert("size", 3);

  ModuleInfo m1{.id = {1}, .type = ModuleType::Fake, .settings = jsonObj1};
  cfg.addModule(m1);

  cfg.addTag({.tag = {4}, .module = m1.id, .channel = {0}});
  cfg.addTag({.tag = {6}, .module = m1.id, .channel = {1}});
  cfg.addTag({.tag = {7}, .module = m1.id, .channel = {2}});

  QJsonObject jsonObj2;
  jsonObj2.insert("size", 2);

  ModuleInfo m2{.id = {2}, .type = ModuleType::Fake, .settings = jsonObj2};
  cfg.addModule(m2);

  cfg.addTag({.tag = {8}, .module = m2.id, .channel = {0}});
  cfg.addTag({.tag = {10}, .module = m2.id, .channel = {1}});

  cfg.addSignalDefinition({.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100});

  cfg.addSignalDefinition({.id = {1}, .name = "Raw1", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 10});

  cfg.addSignalDefinition({.id = {2}, .name = "Raw2", .kind = SignalKind::Raw, .source = {4}, .archiveFrequency = 1000});

  cfg.addSignalDefinition({.id = {3}, .name = "Raw3", .kind = SignalKind::Raw, .source = {6}, .archiveFrequency = 100});

  cfg.addSignalDefinition({.id = {4}, .name = "Raw4", .kind = SignalKind::Raw, .source = {7}, .archiveFrequency = 1});

  cfg.addSignalDefinition({.id = {5}, .name = "Raw5", .kind = SignalKind::Raw, .source = {8}, .archiveFrequency = 100});

  cfg.addSignalDefinition({.id = {6}, .name = "Raw6", .kind = SignalKind::Raw, .source = {10}, .archiveFrequency = 10});

  return cfg;
}

static SystemConfiguration createTestConfig_Fail_ModuleType()
{
  using namespace qds;
  SystemConfiguration cfg;

  QJsonObject jsonObj;
  jsonObj.insert("size", 2);

  ModuleInfo m0{.id = {0}, .type = ModuleType::Fake, .settings = jsonObj};
  cfg.addModule(m0);

  cfg.addTag({.tag = {0}, .module = m0.id, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = m0.id, .channel = {1}});

  ModuleInfo m1{.id = {1}, .type = ModuleType::Failing, .settings = jsonObj};
  cfg.addModule(m1);

  cfg.addTag({.tag = {4}, .module = m1.id, .channel = {0}});
  cfg.addTag({.tag = {6}, .module = m1.id, .channel = {1}});

  ModuleInfo m2{.id = {2}, .type = ModuleType::Fake, .settings = jsonObj};
  cfg.addModule(m2);

  cfg.addTag({.tag = {8}, .module = m2.id, .channel = {0}});
  cfg.addTag({.tag = {10}, .module = m2.id, .channel = {1}});

  cfg.addSignalDefinition({.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100});

  cfg.addSignalDefinition({.id = {1}, .name = "Raw1", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 10});

  cfg.addSignalDefinition({.id = {2}, .name = "Raw2", .kind = SignalKind::Raw, .source = {4}, .archiveFrequency = 1000});

  cfg.addSignalDefinition({.id = {3}, .name = "Raw3", .kind = SignalKind::Raw, .source = {6}, .archiveFrequency = 100});

  cfg.addSignalDefinition({.id = {5}, .name = "Raw5", .kind = SignalKind::Raw, .source = {8}, .archiveFrequency = 100});

  cfg.addSignalDefinition({.id = {6}, .name = "Raw6", .kind = SignalKind::Raw, .source = {10}, .archiveFrequency = 10});

  return cfg;
}

static SystemConfiguration createTestConfig_Fail_DataSource()
{
  using namespace qds;
  SystemConfiguration cfg;

  QJsonObject jsonObj;
  jsonObj.insert("size", 2);

  ModuleInfo m0{.id = {0}, .type = ModuleType::Fake, .settings = jsonObj};
  cfg.addModule(m0);

  cfg.addTag({.tag = {0}, .module = m0.id, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = m0.id, .channel = {1}});

  ModuleInfo m1{.id = {1}, .type = ModuleType::LTR11, .settings = jsonObj};
  cfg.addModule(m1);

  cfg.addTag({.tag = {4}, .module = m1.id, .channel = {0}});
  cfg.addTag({.tag = {6}, .module = m1.id, .channel = {1}});

  ModuleInfo m2{.id = {2}, .type = ModuleType::Fake, .settings = jsonObj};
  cfg.addModule(m2);

  cfg.addTag({.tag = {8}, .module = m2.id, .channel = {0}});
  cfg.addTag({.tag = {10}, .module = m2.id, .channel = {1}});

  cfg.addSignalDefinition({.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100});

  cfg.addSignalDefinition({.id = {1}, .name = "Raw1", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 10});

  cfg.addSignalDefinition({.id = {2}, .name = "Raw2", .kind = SignalKind::Raw, .source = {4}, .archiveFrequency = 1000});

  cfg.addSignalDefinition({.id = {3}, .name = "Raw3", .kind = SignalKind::Raw, .source = {6}, .archiveFrequency = 100});

  cfg.addSignalDefinition({.id = {5}, .name = "Raw5", .kind = SignalKind::Raw, .source = {8}, .archiveFrequency = 100});

  cfg.addSignalDefinition({.id = {6}, .name = "Raw6", .kind = SignalKind::Raw, .source = {10}, .archiveFrequency = 10});

  return cfg;
}

static SystemConfiguration createTestConfigUnknownDependency()
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = {0}, .channel = {1}});

  SignalDefinition sd0 {.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 1000};
  cfg.addSignalDefinition(sd0);
  SignalDefinition sd1 {.id = {1}, .name = "Raw1", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd1);

  SignalDefinition sd2 {.id = {7}, .name = "A", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{0}}};
  cfg.addSignalDefinition(sd2);
  SignalDefinition sd3 {.id = {4}, .name = "B", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {0}, .dependencies = {{1}}};
  cfg.addSignalDefinition(sd3);

  SignalDefinition sd4 {.id = {11}, .name = "C", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {0}, .dependencies = {{4}, {8}}};
  cfg.addSignalDefinition(sd4);

  return cfg;
}

