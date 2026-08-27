#include <QCoreApplication>
#include <QTest>

#include "tst_configuration.h"
#include "tst_dataarchive.h"
#include "tst_database.h"
#include "tst_dataprotocol.h"
#include "tst_common.h"
#include "tst_core.h"
#include "tst_dataserver.h"
#include "tst_datasource.h"
#include "tst_engine.h"
#include "tst_formulas.h"
#include "tst_hardware.h"
#include "tst_packetdispatcher.h"
#include "tst_publisher.h"
#include "tst_signalprocessor.h"
#include "tst_signalstorage.h"
#include "tst_udpsender.h"
#include "tst_subscriptions.h"
//#include "tst_livescheduler.h"
//#include "tst_livestorage.h"
#include "tst_udpserver.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  int rc = 0;

  {
    tst_database tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_dataarchive tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_dataserver tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    test_dataprotocol tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_publisher tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_formulas tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_signalprocessor tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_configuration tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_core tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    test_common tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_subscriptions tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_udpsender tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_datasource tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_engine tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_hardware tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_udpserver tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_packetdispatcher tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_signalstorage tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }

  return rc;
}
