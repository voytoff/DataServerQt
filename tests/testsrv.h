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

static SystemConfiguration createTestConfig(const qds::TagId* tags, int tagCount)
{
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


#endif // TESTSRV_H
