#include "testsrv.h"

TestSrv::TestSrv(const SystemConfiguration& cfg, bool udp, QObject* parent)
  : QObject(parent)
  , dispatcher(cfg, manager, udp ? (ISender&)sender : (ISender&)testSender)
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