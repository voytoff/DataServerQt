#include "testsrv.h"

TestSrv::TestSrv(const SystemConfiguration& cfg, QObject* parent)
  : QObject(parent)
  , dispatcher(cfg, manager, udpSender)
  , server(dispatcher)
{
  layout.build(cfg);
  publisher =
    std::make_unique<Publisher>(
      layout,
      subscriptions,
      sender,
      1000);
}