#ifndef TESTSRV_H
#define TESTSRV_H

#include <QObject>
#include "isender.h"
#include "publisher.h"
#include "subscriptionmanager.h"
#include "livescheduler.h"
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

#endif // TESTSRV_H
