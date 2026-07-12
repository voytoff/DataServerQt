#include <QCoreApplication>
#include <QTest>

#include "tst_common.h"
#include "tst_udpsender.h"
//#include "tst_dataprotocol.h"
//#include "tst_livescheduler.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  int rc = 0;

  {
    test_common tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_udpsender tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }

  return rc;
}