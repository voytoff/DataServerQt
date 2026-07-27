#include "tst_dataarchive.h"
#include "archiveformat.h"
#include <QtTest/qtestcase.h>

tst_dataarchive::tst_dataarchive() { }
tst_dataarchive::~tst_dataarchive() = default;

void tst_dataarchive::test_dataFileHeader_headerSize()
{
  using namespace qds;
  DataFileHeader hdr;

  uint32_t size1 = hdr.headerSize;
  uint32_t size2 = sizeof(hdr);

  QCOMPARE(size1, size2);
}
