#include "tst_dataarchive.h"
#include "archivefile.h"
#include "archiveformat.h"
#include <QtTest/qtestcase.h>

tst_dataarchive::tst_dataarchive() { }
tst_dataarchive::~tst_dataarchive() = default;

static std::string getCurrentFolder() {
  return std::filesystem::current_path().generic_string();
}
static std::string getFilePath(std::string fileName) {
  return std::format("{0}/{1}", getCurrentFolder(), fileName);
}
static qds::DataFileHeader getDataFileHeader() {
  auto recordSize = static_cast<uint32_t>(sizeof(qds::SampleRecordHeader) + 32 * sizeof(float));
  qds::DataFileHeader hdr{
    .module = {999},
    .sampleFrequency = 100,
    .channelCount = 32,
    .recordSize = recordSize,
    .firstTimestamp = 1234567
  };
  return hdr;
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
  std::string filePath = getFilePath(fileName);
  auto recordSize = static_cast<uint32_t>(sizeof(SampleRecordHeader) + 32 * sizeof(float));
  DataFileHeader hdr = getDataFileHeader();

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

  QCOMPARE(rh2.timestamp, 7654321u);

  std::array<float, 32> values2;

  QVERIFY(file.readArray(values2.data(), values2.size()));
  QCOMPARE(file.position(), file.fileSize());

  for (int n = 0; n < 32; ++n) {
    QCOMPARE(values2[n], n);
  }

  // проверка OpenMode::Read
  QVERIFY(!file.saveHeader());
  SampleRecordHeader hr3;
  QVERIFY(!file.writeObject(hr3));

  QCOMPARE(file.position(), file.fileSize());
  QVERIFY(!file.eof());

  QVERIFY(file.seek(file.fileSize()));
  QVERIFY(!file.eof());

  QVERIFY(file.seek(file.fileSize() + 100));

  SampleRecordHeader dummy;
  QVERIFY(!file.readObject(dummy));

  QVERIFY(file.fail());

  QVERIFY(file.seek(HeaderSize));
  QCOMPARE(file.position(), HeaderSize);

  QVERIFY(file.seek(file.fileSize()));
  QVERIFY(!file.eof());

  // проверяем закрытый файл.
  ArchiveFile file1;

  QVERIFY(!file1.seek(0));
  QVERIFY(!file1.flush());
  QVERIFY(!file1.fileSize());
  QVERIFY(!file1.writeObject(dummy));
  QVERIFY(!file1.readObject(dummy));
}

void tst_dataarchive::test_archiveFile_emptyFile()
{
  using namespace qds;
  std::fstream stream;
  std::string filePath = getFilePath(fileName);

  stream.open(filePath, std::ios::binary | std::ios::out | std::ios::trunc);
  QVERIFY(stream);
  QVERIFY(stream.flush());
  stream.close();
  QVERIFY(!stream.is_open());

  ArchiveFile file;
  QVERIFY(!file.open(filePath, OpenMode::Read));
}

void tst_dataarchive::test_archiveFile_badMagic()
{
  using namespace qds;
  DataFileHeader hdr = getDataFileHeader();
  hdr.magic = 0xFF;

  std::string filePath = getFilePath(fileName);

  ArchiveFile file;
  QVERIFY(!file.create(filePath, hdr));
}

void tst_dataarchive::test_archiveFile_invalidVersion()
{
  using namespace qds;
  DataFileHeader hdr = getDataFileHeader();
  hdr.version = 2;

  std::string filePath = getFilePath(fileName);

  ArchiveFile file;
  QVERIFY(!file.create(filePath, hdr));
}

void tst_dataarchive::test_archiveFile_invalidRecordSize()
{
  using namespace qds;
  DataFileHeader hdr = getDataFileHeader();
  hdr.channelCount = 16;
  hdr.recordSize = static_cast<uint32_t>(sizeof(qds::SampleRecordHeader) + 10 * sizeof(float));

  std::string filePath = getFilePath(fileName);

  ArchiveFile file;
  QVERIFY(!file.create(filePath, hdr));
}

void tst_dataarchive::test_archiveFile_invalidHeaderSize()
{
  using namespace qds;
  DataFileHeader hdr = getDataFileHeader();
  hdr.headerSize = hdr.headerSize - 8;

  std::string filePath = getFilePath(fileName);

  ArchiveFile file;
  QVERIFY(!file.create(filePath, hdr));
}

void tst_dataarchive::test_archiveFile_badChannelCount()
{
  using namespace qds;
  DataFileHeader hdr = getDataFileHeader();
  hdr.channelCount = 0;

  std::string filePath = getFilePath(fileName);

  ArchiveFile file;
  QVERIFY(!file.create(filePath, hdr));
}

void tst_dataarchive::test_archiveFile_reCreates()
{
  using namespace qds;
  DataFileHeader hdr = getDataFileHeader();

  std::string filePath = getFilePath(fileName);

  ArchiveFile file;
  QVERIFY(file.create(filePath, hdr));
  QVERIFY(file.create(filePath, hdr));
  QVERIFY(file.create(filePath, hdr));
}

void tst_dataarchive::test_archiveFile_saveHeader()
{
  using namespace qds;
  DataFileHeader hdr = getDataFileHeader();
  QCOMPARE(hdr.lastTimestamp, 0);

  std::string filePath = getFilePath(fileName);

  ArchiveFile file;
  QVERIFY(file.create(filePath, hdr));
  QCOMPARE(file.header().lastTimestamp, 0);

  file.setLastTimestamp(777);
  QVERIFY(file.saveHeader());
  QCOMPARE(file.header().lastTimestamp, 777);

  file.close();
  QVERIFY(!file.isOpen());

  QVERIFY(file.open(filePath, OpenMode::ReadWrite));

  QCOMPARE(file.header().lastTimestamp, 777);

  file.setLastTimestamp(12345678);
  QVERIFY(file.saveHeader());
  QCOMPARE(file.header().lastTimestamp, 12345678);

  file.close();
  QVERIFY(!file.isOpen());


  QVERIFY(file.open(filePath, OpenMode::Read));

  QCOMPARE(file.header().lastTimestamp, 12345678);
}
