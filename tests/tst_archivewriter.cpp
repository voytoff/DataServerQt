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

  std::array<float, 32> values;
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

  QCOMPARE(file.header().recordCount, 1u);
  QCOMPARE(file.header().lastTimestamp, timestamp);

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

void tst_archivewriter::test_archivewriter_writeSomeRecords()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = tst_dataarchive::getFilePath(fileName);
  DataFileHeader hdr = tst_dataarchive::getDataFileHeader();

  QVERIFY(writer.open(filePath, hdr));

  QVERIFY(writer.isOpen());

  std::array<float, 32> values;

  auto recordSize = static_cast<uint32_t>(sizeof(SampleRecordHeader) + values.size() * sizeof(float));

  float value = 0;

  for (int i = 0; i < 10; i++) {

    for (int n = 0; n < values.size(); ++n) {
      values[n] = ++value;
    }

    QVERIFY(writer.write(i, values));
  }

  writer.close();
  QVERIFY(!writer.isOpen());

  ArchiveFile file;
  QVERIFY(file.open(filePath, OpenMode::Read));

  QCOMPARE(file.header().recordCount, 10u);
  QCOMPARE(file.header().lastTimestamp, 9u);

  value = 0;
  for (int i = 0; i < 10; i++) {
    SampleRecordHeader rh;
    QVERIFY(file.readObject(rh));

    QCOMPARE(rh.timestamp, i);

    std::array<float, values.size()> values2;

    QVERIFY(file.readArray(values2.data(), values2.size()));

    for (int n = 0; n < values2.size(); ++n) {
      QCOMPARE(values2[n], ++value);
    }
  }


  file.close();
  QVERIFY(!file.isOpen());
}

void tst_archivewriter::test_archivewriter_badChannelCount()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = tst_dataarchive::getFilePath(fileName);
  DataFileHeader hdr = tst_dataarchive::getDataFileHeader();

  QVERIFY(writer.open(filePath, hdr));

  QVERIFY(writer.isOpen());

  std::array<float, 31> values;
  QVERIFY(!writer.write(777, values));

  QCOMPARE(writer.recordCount(), 0u);

  QCOMPARE(writer.fileSize(), HeaderSize);


  std::array<float, 33> values2;
  QVERIFY(!writer.write(999, values2));

  QCOMPARE(writer.recordCount(), 0u);

  QCOMPARE(writer.fileSize(), HeaderSize);


  writer.close();
  QVERIFY(!writer.isOpen());
}

void tst_archivewriter::test_archivewriter_writeAfterClose()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = tst_dataarchive::getFilePath(fileName);
  DataFileHeader hdr = tst_dataarchive::getDataFileHeader();

  QVERIFY(writer.open(filePath, hdr));

  QVERIFY(writer.isOpen());

  std::array<float, 32> values;
  QVERIFY(writer.write(777, values));
  QCOMPARE(writer.recordCount(), 1u);

  writer.close();
  QVERIFY(!writer.isOpen());

  std::array<float, 32> values2;
  QVERIFY(!writer.write(999, values2));
  QCOMPARE(writer.recordCount(), 1u);

  writer.close();
  QVERIFY(!writer.isOpen());

  ArchiveFile file;
  QVERIFY(file.open(filePath, OpenMode::Read));
  QCOMPARE(file.header().recordCount, 1u);

  file.close();
  QVERIFY(!file.isOpen());
}

void tst_archivewriter::test_archivewriter_doubleClose()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = tst_dataarchive::getFilePath(fileName);
  DataFileHeader hdr = tst_dataarchive::getDataFileHeader();

  QVERIFY(writer.open(filePath, hdr));

  QVERIFY(writer.isOpen());

  std::array<float, 32> values;
  QVERIFY(!writer.write(0, values));

  writer.close();
  QVERIFY(!writer.isOpen());

  writer.close();
  QVERIFY(!writer.isOpen());
}
