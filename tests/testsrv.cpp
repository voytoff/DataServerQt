#include "testsrv.h"

TestSrv::TestSrv(const SystemConfiguration& cfg, QObject* parent)
  : QObject(parent)
  , publisher(layout, manager, udpSender, 1000)
  , dispatcher(cfg, manager, udpSender)
  , server(dispatcher)
{
  layout.build(cfg);
}