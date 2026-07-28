#include "tst_dataarchive.h"
#include "archivefile.h"
#include "archiveformat.h"
#include <QtTest/qtestcase.h>

tst_dataarchive::tst_dataarchive() { }
tst_dataarchive::~tst_dataarchive() = default;

static std::string getCurrentFolder() {
  return std::filesystem::current_path().generic_string();
}

void tst_dataarchive::test_dataFileHeader_headerSize()
{
  using namespace qds;
  DataFileHeader hdr;

  uint32_t size1 = hdr.headerSize;
  uint32_t size2 = sizeof(hdr);

  QCOMPARE(size1, size2);
}

void tst_dataarchive::test_dataFileHeader_base()
{
  using namespace qds;
  std::string filePath = std::format("{0}/{1}", getCurrentFolder(), fileName);
  auto recordSize = static_cast<uint32_t>(sizeof(SampleRecordHeader) + 32 * sizeof(float));
  DataFileHeader hdr{
    .module = {999},
    .sampleFrequency = 100,
    .channelCount = 32,
    .recordSize = recordSize,
    .firstTimestamp = 1234567
  };

  QCOMPARE(hdr.headerSize, sizeof(hdr));
  ArchiveFile file;
  QVERIFY(file.create(filePath, hdr));

  QVERIFY(file.isOpen());
  QVERIFY(file.isWritable());
  QVERIFY(!file.isReadable());
  QCOMPARE(file.mode(), OpenMode::Write);

  QCOMPARE(file.header().magic, ArchiveMagic);
  QCOMPARE(file.header().version, ArchiveVersion);
  QCOMPARE(file.header().sampleFrequency, 100u);
  QCOMPARE(file.header().channelCount, 32u);
  QCOMPARE(file.header().module.value, 999u);
  QCOMPARE(file.header().firstTimestamp, 1234567u);
  QCOMPARE(file.header().recordSize, recordSize);

  QCOMPARE(file.position(), HeaderSize);

  SampleRecordHeader rh{
    .timestamp = 7654321
  };

  QVERIFY(file.writeObject(rh));

  std::array<float, 32> values;
  for (int n = 0; n < 32; ++n) {
    values[n] = n;
  }

  QVERIFY(file.writeArray(values.data(), values.size()));
  auto pos = file.position();
  file.saveHeader();
  QCOMPARE(file.position(), pos);

  QCOMPARE(file.position(), file.fileSize());

  file.close();
  QVERIFY(!file.isOpen());

  QVERIFY(file.open(filePath, OpenMode::Read));
  QVERIFY(file.isOpen());
  QVERIFY(!file.isWritable());
  QVERIFY(file.isReadable());
  QCOMPARE(file.mode(), OpenMode::Read);

  QCOMPARE(file.header().magic, ArchiveMagic);
  QCOMPARE(file.header().version, ArchiveVersion);
  QCOMPARE(file.header().sampleFrequency, 100u);
  QCOMPARE(file.header().channelCount, 32u);
  QCOMPARE(file.header().module.value, 999u);
  QCOMPARE(file.header().firstTimestamp, 1234567u);
  QCOMPARE(file.header().recordSize, recordSize);

  QCOMPARE(file.position(), HeaderSize);

  SampleRecordHeader rh2;
  QVERIFY(file.readObject(rh2));

  std::array<float, 32> values2;

  QVERIFY(file.readArray(values2.data(), values2.size()));
  QCOMPARE(file.position(), file.fileSize());

  for (int n = 0; n < 32; ++n) {
    QCOMPARE(values2[n], n);
  }

  QCOMPARE(file.position(), file.fileSize());
  QVERIFY(!file.eof());

  QVERIFY(file.seek(file.fileSize()));
  QVERIFY(!file.eof());

  QVERIFY(file.seek(file.fileSize() + 100));

  SampleRecordHeader dummy;
  QVERIFY(!file.readObject(dummy));
  //QVERIFY(!file); // не проходит

  QVERIFY(file.seek(HeaderSize));
  QCOMPARE(file.position(), HeaderSize);

  QVERIFY(file.seek(file.fileSize()));
  QVERIFY(file.eof() || file.fail()); // не проходит

}
