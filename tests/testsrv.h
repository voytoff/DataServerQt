#ifndef TESTSRV_H
#define TESTSRV_H

#include <QObject>
#include "isender.h"
#include "moduleinfo.h"
#include "publisher.h"
#include "subscriptionmanager.h"
#include "livescheduler.h"
#include "systemconfiguration.h"
#include "taginfo.h"
#include "udpserver.h"

using namespace qds;
class TestSrv : public QObject
{
  Q_OBJECT
public:
  explicit TestSrv(const SystemConfiguration& cfg, QObject *parent = nullptr);

  SubscriptionManager manager;
  Publisher publisher;
  LiveStorage storage;
  TestSender sender;
  LiveScheduler scheduler;
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

#endif // TESTSRV_H
