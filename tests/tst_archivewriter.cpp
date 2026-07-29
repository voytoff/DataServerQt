#include "tst_archivewriter.h"
#include "archivewriter.h"
#include "tst_dataarchive.h"
#include <qtestcase.h>

tst_archivewriter::tst_archivewriter() { }
tst_archivewriter::~tst_archivewriter() = default;

void tst_archivewriter::test_archivewriter_create_open()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = tst_dataarchive::getFilePath(fileName);
  DataFileHeader hdr = tst_dataarchive::getDataFileHeader();

  QVERIFY(writer.open(filePath, hdr));

  QVERIFY(writer.isOpen());

  writer.close();
  QVERIFY(!writer.isOpen());
}

void tst_archivewriter::test_archivewriter_writeOneRecord()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = tst_dataarchive::getFilePath(fileName);
  DataFileHeader hdr = tst_dataarchive::getDataFileHeader();

  QVERIFY(writer.open(filePath, hdr));

  QVERIFY(writer.isOpen());

  std::array<float, 32/*hdr.channelCount*/> values;
  for (float n = 0; n < values.size(); ++n) {
    values[n] = n;
  }
  auto recordSize = static_cast<uint32_t>(sizeof(SampleRecordHeader) + values.size() * sizeof(float));

  QVERIFY(writer.write(timestamp, values));

  writer.close();
  QVERIFY(!writer.isOpen());

  ArchiveFile file;
  QVERIFY(file.open(filePath, OpenMode::Read));

  QCOMPARE(file.header().magic, ArchiveMagic);
  QCOMPARE(file.header().version, ArchiveVersion);
  QCOMPARE(file.header().sampleFrequency, 100u);
  QCOMPARE(file.header().channelCount, 32u);
  QCOMPARE(file.header().module.value, 999u);
  QCOMPARE(file.header().firstTimestamp, firstTimestamp);
  QCOMPARE(file.header().recordSize, recordSize);

  SampleRecordHeader rh;
  QVERIFY(file.readObject(rh));
  QCOMPARE(rh.timestamp, timestamp);

  std::array<float, 32> values2;
  QVERIFY(file.readArray(values2.data(), values2.size()));

  for (float n = 0; n < values.size(); ++n) {
    QCOMPARE(values2[n], n);
  }

  file.close();
  QVERIFY(!file.isOpen());
}
