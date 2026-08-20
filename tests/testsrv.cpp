#include "testsrv.h"

TestSrv::TestSrv(const SystemConfiguration& cfg, QObject* parent)
  : QObject(parent)
  , storage(cfg)
  , publisher(layout, manager, udpSender, 1000)
  , scheduler(storage, manager, publisher, publisherSender)
  , dispatcher(cfg, manager, scheduler, udpSender)
  , server(dispatcher)
{
  layout.build(cfg);
}