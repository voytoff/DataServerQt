#pragma once

#include <QObject>
#include "isender.h"
#include "moduleinfo.h"
#include "publisher.h"
#include "subscriptionmanager.h"
#include "livescheduler.h"
#include "systemconfiguration.h"
#include "taginfo.h"
#include "udpsender.h"
#include "udpserver.h"

using namespace qds;
class TestSrv : public QObject
{
  Q_OBJECT
public:
  explicit TestSrv(const SystemConfiguration& cfg, QObject *parent = nullptr);

  SubscriptionManager manager;

  LiveStorage storage;

  Publisher publisher;

  TestPublisherSender publisherSender;
  UdpSender udpSender;

  LiveScheduler scheduler;

  PacketDispatcher dispatcher;

  UdpServer server;
};

static SystemConfiguration createTestConfig(const qds::TagId* tags, int tagCount) {
  using namespace qds;
  // создаем конфигурацию
  SystemConfiguration cfg;

  ModuleInfo m0;
  m0.id = {0};
  cfg.addModule(m0);

  for (int i = 0; i < tagCount; i++) {
    TagId t = tags[i];
    TagInfo ti;
    ti.tag = t;
    ti.module = {0};
    cfg.addTag(ti);
  }

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

static qds::SystemConfiguration createTestConfigWithSignalDefinitions() {
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{0};
  cfg.addModule(m);

  TagInfo t1{.tag = {0}, .module = {0}, .channel = {0}};
  cfg.addTag(t1);

  TagInfo t2{.tag = {1}, .module = {0}, .channel = {0}};
  cfg.addTag(t2);

  SignalDefinition sd1 {.id = 0, .name = "U000", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd1);

  SignalDefinition sd2 {.id = 1, .name = "D10", .kind = SignalKind::Calculated, .source = {0}, .archiveFrequency = 10, .calibrationId = 72};
  cfg.addSignalDefinition(sd2);

  SignalDefinition sd3 {.id = 2, .name = "P100", .kind = SignalKind::Calculated, .source = {0}, .archiveFrequency = 100, .formulaId = 15};
  cfg.addSignalDefinition(sd3);

  SignalDefinition sd4 {.id = 3, .name = "U001", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 10};
  cfg.addSignalDefinition(sd4);

  return cfg;
}

