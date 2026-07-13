#include <QCoreApplication>
#include <QTest>

#include "tst_common.h"
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

  return rc;
}