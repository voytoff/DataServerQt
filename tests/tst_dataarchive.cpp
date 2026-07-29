#include "tst_dataarchive.h"
#include "archivefile.h"
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

void tst_dataarchive::test_dataFileHeader_base()
{
  using namespace qds;
  std::string filePath = getFilePath(fileName);
  DataFileHeader hdr = getDataFileHeader();
  auto recordSize = static_cast<uint32_t>(sizeof(SampleRecordHeader) + hdr.channelCount * sizeof(float));

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
  QCOMPARE(file.header().firstTimestamp, firstTimestamp);
  QCOMPARE(file.header().recordSize, recordSize);

  QCOMPARE(file.position(), HeaderSize);

  SampleRecordHeader rh{
    .timestamp = timestamp
  };

  QVERIFY(file.writeObject(rh));

  std::array<float, 32> values;
  for (float n = 0; n < values.size(); ++n) {
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
  QCOMPARE(file.header().firstTimestamp, firstTimestamp);
  QCOMPARE(file.header().recordSize, recordSize);

  QCOMPARE(file.position(), HeaderSize);

  SampleRecordHeader rh2;
  QVERIFY(file.readObject(rh2));

  QCOMPARE(rh2.timestamp, timestamp);

  std::array<float, 32> values2;

  QVERIFY(file.readArray(values2.data(), values2.size()));
  QCOMPARE(file.position(), file.fileSize());

  for (float n = 0; n < values.size(); ++n) {
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

  file.close();
  QVERIFY(!file.isOpen());
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

  file.close();
  QVERIFY(!file.isOpen());
}

void tst_dataarchive::test_archiveFile_saveHeader()
{
  using namespace qds;
  ArchiveFile file = getAfterCreateArchiveFile();
  QVERIFY(file);

  auto filePath = file.path();

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

  file.close();
  QVERIFY(!file.isOpen());
}

void tst_dataarchive::test_archiveFile_saveHeaderInReadMode()
{
  using namespace qds;
  ArchiveFile file = getAfterCreateArchiveFile();
  QVERIFY(file);

  auto filePath = file.path();

  file.close();
  QVERIFY(!file.isOpen());

  QVERIFY(file.open(filePath, OpenMode::Read));

  QVERIFY(!file.saveHeader());

  file.close();
  QVERIFY(!file.isOpen());
}

void tst_dataarchive::test_archiveFile_seekToZerro()
{
  using namespace qds;
  ArchiveFile file = getAfterCreateArchiveFile();
  QVERIFY(file);

  QVERIFY(file.seek(0));
  QCOMPARE(file.position() , 0);

  auto filePath = file.path();

  file.close();
  QVERIFY(!file.isOpen());

  QVERIFY(file.open(filePath, OpenMode::Read));
  QCOMPARE(file.position(), HeaderSize);

  QVERIFY(file.seek(0));
  QCOMPARE(file.position() , 0);

  DataFileHeader hdr;
  QVERIFY(file.readObject(hdr));

  QCOMPARE(hdr.magic, ArchiveMagic);
  QCOMPARE(hdr.version, ArchiveVersion);

  file.close();
  QVERIFY(!file.isOpen());
}

void tst_dataarchive::test_archiveFile_seekToDataAfterHeader()
{
  using namespace qds;
  ArchiveFile file = getAfterCreateArchiveFile();
  QVERIFY(file);

  QCOMPARE(file.position(), HeaderSize);

  SampleRecordHeader rh{
    .timestamp = timestamp
  };

  QVERIFY(file.writeObject(rh));

  std::array<float, 4> values;
  for (float n = 0; n < values.size(); ++n) {
    values[n] = n;
  }

  QVERIFY(file.writeArray(values.data(), values.size()));
  auto pos = file.position();

  auto filePath = file.path();

  file.close();
  QVERIFY(!file.isOpen());

  QVERIFY(file.open(filePath, OpenMode::Read));
  QCOMPARE(file.position(), HeaderSize);

  QVERIFY(file.seek(0));
  QCOMPARE(file.position() , 0);

  QVERIFY(file.seek(HeaderSize));
  QCOMPARE(file.position() , HeaderSize);

  SampleRecordHeader rh2;
  QVERIFY(file.readObject(rh2));

  QCOMPARE(rh2.timestamp, timestamp);

  std::array<float, values.size()> values2;

  QVERIFY(file.readArray(values2.data(), values2.size()));
  QCOMPARE(file.position(), file.fileSize());

  for (float n = 0; n < values2.size(); ++n) {
    QCOMPARE(values2[n], n);
  }

  QCOMPARE(file.position(), pos);

  file.close();
  QVERIFY(!file.isOpen());
}

void tst_dataarchive::test_archiveFile_saveZerroArray()
{
  using namespace qds;
  ArchiveFile file = getAfterCreateArchiveFile();
  QVERIFY(file);

  QCOMPARE(file.position(), HeaderSize);

  SampleRecordHeader rh{
    .timestamp = timestamp
  };

  QVERIFY(file.writeObject(rh));

  std::array<float, 0> values;

  QVERIFY(file.writeArray(values.data(), values.size()));

  QCOMPARE(file.position(), HeaderSize + sizeof(SampleRecordHeader));


  file.close();
  QVERIFY(!file.isOpen());
}

void tst_dataarchive::test_archiveFile_checkFileSize()
{
  using namespace qds;
  ArchiveFile file = getAfterCreateArchiveFile();
  QVERIFY(file);

  QCOMPARE(file.position(), HeaderSize);

  SampleRecordHeader rh{
    .timestamp = timestamp
  };

  QVERIFY(file.writeObject(rh));

  std::array<float, 32> values;

  QVERIFY(file.writeArray(values.data(), values.size()));

  auto fileSize = HeaderSize + sizeof(SampleRecordHeader) + (values.size() * sizeof(float));

  QCOMPARE(file.position(), fileSize);
  QCOMPARE(file.fileSize(), fileSize);


  file.close();
  QVERIFY(!file.isOpen());
}

void tst_dataarchive::test_archiveFile_saveHeaderNotChangePosition()
{
  using namespace qds;
  ArchiveFile file = getAfterCreateArchiveFile();
  QVERIFY(file);

  QCOMPARE(file.position(), HeaderSize);

  SampleRecordHeader rh{
    .timestamp = timestamp
  };

  QVERIFY(file.writeObject(rh));

  std::array<float, 32> values;

  QVERIFY(file.writeArray(values.data(), values.size()));

  auto pos = file.position();

  file.setFirstTimestamp(0xFFFF);
  file.saveHeader();

  QCOMPARE(file.position(), pos);


  file.close();
  QVERIFY(!file.isOpen());
}
