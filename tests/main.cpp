#include <QCoreApplication>
#include <QTest>

#include "test_dataprotocol.h"
#include "tst_common.h"
#include "tst_core.h"
#include "tst_datasource.h"
#include "tst_publisher.h"
#include "tst_udpsender.h"
#include "tst_subscriptions.h"
#include "tst_livescheduler.h"
#include "tst_livestorage.h"
#include "tst_udpserver.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  int rc = 0;

  {
    tst_core tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    test_common tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_livescheduler tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_livestorage tc;
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
    tst_udpserver tc;
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
    tst_datasource tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }


  return rc;
}