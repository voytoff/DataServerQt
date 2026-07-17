#include "testsrv.h"

TestSrv::TestSrv(const SystemConfiguration& cfg, QObject* parent)
  : QObject(parent)
  , storage(cfg)
  , scheduler(storage, manager, publisher, sender)
  , server(cfg, manager, scheduler)
{
}