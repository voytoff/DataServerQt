#include <QTimer>
#include <QCoreApplication>
#include "dataengine.h"
#include "datasourcemanager.h"
#include "livescheduler.h"
#include "publisher.h"
#include "subscriptionmanager.h"
#include "systemconfiguration.h"
#include "udpsender.h"

int main(int argc, char *argv[]) {
  using namespace qds;
  SystemConfiguration config;

  LiveStorage storage(config);

  SubscriptionManager subscriptions;

  UdpSender sender;

  Publisher publisher(
    storage,
    sender);

  LiveScheduler scheduler(
    subscriptions,
    publisher);

  DataSourceManager sources;

  DataEngine engine(
    sources,
    scheduler);

  auto _ = engine.start();

  QCoreApplication app(argc, argv);

  QTimer timer;

  QAbstractEventDispatcher::connect(
    &timer,
    &QTimer::timeout,
    [&]()
    {
      auto _ = engine.step();
    });

  timer.start(1);

  return app.exec();
}