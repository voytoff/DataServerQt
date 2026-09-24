#include "tst_dataarchive.h"
#include "archivedescription.h"
#include "archivedescriptionbuilder.h"
#include "archivedescriptionwriter.h"
#include "archivefile.h"
#include "archiveformat.h"
#include "archivewriter.h"
#include "buffermanager.h"
#include "datablockqueue.h"
#include "datastreamprocessor.h"
#include "datastreamreader.h"
#include "datastreamtime.h"
#include "datastreamworker.h"
#include "fakeclock.h"
#include "framestartpolicy.h"
#include "fakedatastreameventsink.h"
#include "fakelcardmodule.h"
#include "frameassembler.h"
#include "lcarddatasource.h"
#include "moduletype.h"
#include "qds/testarchiveframewriter.h"
#include "signalprocessor.h"
#include "smartblocklcardmodule.h"
#include "testlogger.h"
#include "testsrv.h"
#include <QtTest/qtestcase.h>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <array>
#include <qtestsupport_core.h>

tst_dataarchive::tst_dataarchive() { }
tst_dataarchive::~tst_dataarchive() = default;


void tst_dataarchive::test_archivewriter_create_open()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = getFilePath(fileName);
  DataFileHeader hdr = getDataFileHeader();

  QVERIFY(writer.open(filePath, hdr));

  QVERIFY(writer.isOpen());

  writer.close();
  QVERIFY(!writer.isOpen());
}

void tst_dataarchive::test_archivewriter_writeOneRecord()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = getFilePath(fileName);
  DataFileHeader hdr = getDataFileHeader();

  QVERIFY(writer.open(filePath, hdr));

  QVERIFY(writer.isOpen());

  std::array<float, 32> values;
  for (int n = 0; n < values.size(); ++n) {
    values[n] = n;
  }
  auto recordSize = static_cast<uint32_t>(sizeof(SampleRecordHeader) + values.size() * sizeof(float));

  QVERIFY(writer.write(Timestamp{timestamp}, {0}, {12}, values));

  writer.close();
  QVERIFY(!writer.isOpen());

  ArchiveFile file;
  QVERIFY(file.open(filePath, OpenMode::Read));

  QCOMPARE(file.header().magic, ArchiveMagic);
  QCOMPARE(file.header().version, ArchiveVersion);
  QCOMPARE(file.header().sampleFrequency, 100u);
  QCOMPARE(file.header().channelCount, 32u);
  QCOMPARE(file.header().module.value, 999u);
  QCOMPARE(file.header().firstTimestamp, timestamp);
  QCOMPARE(file.header().recordSize, recordSize);

  QCOMPARE(file.header().recordCount, 1u);
  QCOMPARE(file.header().lastTimestamp, timestamp);

  SampleRecordHeader rh;
  QVERIFY(file.readObject(rh));
  QCOMPARE(rh.timestamp, timestamp);
  QCOMPARE(rh.frameNumber, 0);
  QCOMPARE(rh.wallTime, 12);

  std::array<float, 32> values2;
  QVERIFY(file.readArray(values2.data(), values2.size()));

  for (int n = 0; n < values.size(); ++n) {
    QCOMPARE(values2[n], n);
  }

  file.close();
  QVERIFY(!file.isOpen());
}

void tst_dataarchive::test_archivewriter_writeSomeRecords()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = ::getFilePath(fileName);
  DataFileHeader hdr = ::getDataFileHeader();

  QVERIFY(writer.open(filePath, hdr));

  QVERIFY(writer.isOpen());

  std::array<float, 32> values;

  auto recordSize = static_cast<uint32_t>(sizeof(SampleRecordHeader) + values.size() * sizeof(float));

  float value = 0;

  for (uint i = 0; i < 10; i++) {

    for (int n = 0; n < values.size(); ++n) {
      values[n] = ++value;
    }

    QVERIFY(writer.write({i*2}, {i}, {i*100}, values));
  }

  writer.close();
  QVERIFY(!writer.isOpen());

  ArchiveFile file;
  QVERIFY(file.open(filePath, OpenMode::Read));

  QCOMPARE(file.header().recordCount, 10u);
  QCOMPARE(file.header().lastTimestamp, 9u * 2);

  value = 0;
  for (int i = 0; i < 10; i++) {
    SampleRecordHeader rh;
    QVERIFY(file.readObject(rh));

    QCOMPARE(rh.timestamp, i*2);
    QCOMPARE(rh.frameNumber, i);
    QCOMPARE(rh.wallTime, i*100);

    std::array<float, values.size()> values2;

    QVERIFY(file.readArray(values2.data(), values2.size()));

    for (int n = 0; n < values2.size(); ++n) {
      QCOMPARE(values2[n], ++value);
    }
  }


  file.close();
  QVERIFY(!file.isOpen());
}

void tst_dataarchive::test_archivewriter_badChannelCount()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = ::getFilePath(fileName);
  DataFileHeader hdr = ::getDataFileHeader();

  QVERIFY(writer.open(filePath, hdr));

  QVERIFY(writer.isOpen());

  std::array<float, 31> values;
  QVERIFY(!writer.write({777}, {0}, {111}, values));

  QCOMPARE(writer.recordCount(), 0u);

  QCOMPARE(writer.fileSize(), HeaderSize);


  std::array<float, 33> values2;
  QVERIFY(!writer.write({999}, {1}, {222}, values2));

  QCOMPARE(writer.recordCount(), 0u);

  QCOMPARE(writer.fileSize(), HeaderSize);


  writer.close();
  QVERIFY(!writer.isOpen());
}

void tst_dataarchive::test_archivewriter_writeAfterClose()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = ::getFilePath(fileName);
  DataFileHeader hdr = ::getDataFileHeader();

  QVERIFY(writer.open(filePath, hdr));

  QVERIFY(writer.isOpen());

  std::array<float, 32> values;
  QVERIFY(writer.write({777}, {0}, {111}, values));
  QCOMPARE(writer.recordCount(), 1u);

  writer.close();
  QVERIFY(!writer.isOpen());

  std::array<float, 32> values2;
  QVERIFY(!writer.write({999}, {1}, {222}, values2));
  QCOMPARE(writer.recordCount(), 1u);

  writer.close();
  QVERIFY(!writer.isOpen());

  ArchiveFile file;
  QVERIFY(file.open(filePath, OpenMode::Read));
  QCOMPARE(file.header().recordCount, 1u);

  file.close();
  QVERIFY(!file.isOpen());
}

void tst_dataarchive::test_archivewriter_doubleClose()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = getFilePath(fileName);
  DataFileHeader hdr = getDataFileHeader();

  QVERIFY(writer.open(filePath, hdr));

  QVERIFY(writer.isOpen());

  std::array<float, 32> values;
  QVERIFY(writer.write({0}, {1}, {100}, values));

  writer.close();
  QVERIFY(!writer.isOpen());

  writer.close();
  QVERIFY(!writer.isOpen());
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

void tst_dataarchive::test_archiveDescription_writeRawSignals()
{
  using namespace qds;

  ArchiveDescription description;

  description.version = 1;

  ArchiveFileDescription file;

  file.name =
    "raw_100Hz.dat";

  file.frequency =
    100;

  file.dataType =
    "float";

  file.signalIds =
    {
      {
       .index = 0,
       .id = SignalId{0},
       .name = "Raw0",
       .kind = SignalKind::Raw,
       .module = ModuleId{0},
      .channel = ChannelId{0},
       },
      {
       .index = 1,
       .id = SignalId{1},
       .name = "Raw1",
       .kind = SignalKind::Raw,
       .module = ModuleId{0},
       .channel = ChannelId{1},
       }
    };

  description.files.push_back(file);

  ArchiveDescriptionWriter writer;

  const auto path =
    ::getFilePath(
      "description.json");

  QVERIFY(
    writer.write(
      path,
      description));

  QFile jsonFile(
    QString::fromStdString(path));

  QVERIFY(
    jsonFile.open(
      QIODevice::ReadOnly));

  const QByteArray data =
    jsonFile.readAll();

  QVERIFY(
    !data.isEmpty());

  QJsonParseError error;

  const QJsonDocument document =
    QJsonDocument::fromJson(
      data,
      &error);

  QCOMPARE(
    error.error,
    QJsonParseError::NoError);

  QVERIFY(
    document.isObject());

  const QJsonObject root =
    document.object();

  QCOMPARE(
    root["version"].toInt(),
    1);

  const QJsonArray files =
    root["files"].toArray();

  QCOMPARE(
    files.size(),
    1);

  const QJsonObject jsonFileDescription =
    files[0].toObject();

  QCOMPARE(
    jsonFileDescription["name"].toString(),
    "raw_100Hz.dat");

  QCOMPARE(
    jsonFileDescription["frequency"].toInt(),
    100);

  QCOMPARE(
    jsonFileDescription["dataType"].toString(),
    "float");

  const QJsonArray signalIds =
    jsonFileDescription["signals"].toArray();

  QCOMPARE(
    signalIds.size(),
    2);

  // ------------------------------------------------------------
  // Signal 0
  // ------------------------------------------------------------

  const QJsonObject signal0 =
    signalIds[0].toObject();

  QCOMPARE(
    signal0["index"].toInt(),
    0);

  QCOMPARE(
    signal0["id"].toInt(),
    0);

  QCOMPARE(
    signal0["name"].toString(),
    "Raw0");

  QCOMPARE(
    signal0["kind"].toString(),
    "Raw");

  QVERIFY(
    signal0.contains("module"));

  QCOMPARE(
    signal0["module"].toInt(),
    0);

  QVERIFY(
    signal0.contains("channel"));

  QCOMPARE(
    signal0["channel"].toInt(),
    0);

  // ------------------------------------------------------------
  // Signal 1
  // ------------------------------------------------------------

  const QJsonObject signal1 =
    signalIds[1].toObject();

  QCOMPARE(
    signal1["index"].toInt(),
    1);

  QCOMPARE(
    signal1["id"].toInt(),
    1);

  QCOMPARE(
    signal1["name"].toString(),
    "Raw1");

  QCOMPARE(
    signal1["kind"].toString(),
    "Raw");

  QVERIFY(
    signal1.contains("module"));

  QCOMPARE(
    signal1["module"].toInt(),
    0);

  QVERIFY(
    signal1.contains("channel"));

  QCOMPARE(
    signal1["channel"].toInt(),
    1);
}

void tst_dataarchive::test_archiveDescription_writeCalculatedSignals()
{
  using namespace qds;

  ArchiveDescription description;

  description.version = 1;

  ArchiveFileDescription file;

  file.name =
    "calc_10Hz.dat";

  file.frequency =
    10;

  file.dataType =
    "float";

  file.signalIds =
    {
      {
        .index = 0,
        .id = SignalId{17},
        .name = "A",
        .kind = SignalKind::Calculated,
      },
      {
        .index = 1,
        .id = SignalId{4},
        .name = "B",
        .kind = SignalKind::Calculated,
      }
    };

  description.files.push_back(file);

  ArchiveDescriptionWriter writer;

  const auto path =
    ::getFilePath(
      "description.json");

  QVERIFY(
    writer.write(
      path,
      description));

  QFile jsonFile(
    QString::fromStdString(path));

  QVERIFY(
    jsonFile.open(
      QIODevice::ReadOnly));

  const QByteArray data =
    jsonFile.readAll();

  QVERIFY(
    !data.isEmpty());

  QJsonParseError error;

  const QJsonDocument document =
    QJsonDocument::fromJson(
      data,
      &error);

  QCOMPARE(
    error.error,
    QJsonParseError::NoError);

  QVERIFY(
    document.isObject());

  const QJsonObject root =
    document.object();

  QCOMPARE(
    root["version"].toInt(),
    1);

  const QJsonArray files =
    root["files"].toArray();

  QCOMPARE(
    files.size(),
    1);

  const QJsonObject jsonFileDescription =
    files[0].toObject();

  QCOMPARE(
    jsonFileDescription["name"].toString(),
    "calc_10Hz.dat");

  QCOMPARE(
    jsonFileDescription["frequency"].toInt(),
    10);

  QCOMPARE(
    jsonFileDescription["dataType"].toString(),
    "float");

  const QJsonArray signalIds =
    jsonFileDescription["signals"].toArray();

  QCOMPARE(
    signalIds.size(),
    2);

  const QJsonObject signal0 =
    signalIds[0].toObject();

  QCOMPARE(
    signal0["index"].toInt(),
    0);

  QCOMPARE(
    signal0["id"].toInt(),
    17);

  QCOMPARE(
    signal0["name"].toString(),
    "A");

  QCOMPARE(
    signal0["kind"].toString(),
    "Calculated");

  QCOMPARE(
    signal0.contains("module"), false);

  QCOMPARE(
    signal0.contains("channel"), false);
}

void tst_dataarchive::test_archiveDescription_writeMultipleFiles()
{
  using namespace qds;

  ArchiveDescription description;

  description.version = 1;

  std::array<ArchiveFileDescription, 3> jsonFiles;
  jsonFiles[0] = {
    .name = "raw_100Hz.dat",
    .frequency = 100,
    .dataType = "float",
    .signalIds =
      {
        {
         .index = 0,
         .id = SignalId{0},
         .name = "Raw0",
         .kind = SignalKind::Raw,
         .module = ModuleId{0},
         .channel = ChannelId{0},
        },
        {
         .index = 1,
         .id = SignalId{1},
         .name = "Raw1",
         .kind = SignalKind::Raw,
         .module = ModuleId{0},
         .channel = ChannelId{1},
        }
      }
  };

  description.files.push_back(jsonFiles[0]);

  jsonFiles[1] = {
    .name = "calculated_10Hz.dat",
    .frequency = 10,
    .dataType = "float",
    .signalIds =
      {
        {
         .index = 0,
         .id = SignalId{17},
         .name = "A",
         .kind = SignalKind::Calculated,
        },
        {
         .index = 1,
         .id = SignalId{4},
         .name = "B",
         .kind = SignalKind::Calculated,
        }
      }
  };

  description.files.push_back(jsonFiles[1]);

  jsonFiles[2] = {
    .name = "calculated_100Hz.dat",
    .frequency = 100,
    .dataType = "float",
    .signalIds =
    {
      {
       .index = 0,
       .id = SignalId{23},
       .name = "C",
       .kind = SignalKind::Calculated,
      },
    }
  };

  description.files.push_back(jsonFiles[2]);

  ArchiveDescriptionWriter writer;

  const auto path =
    ::getFilePath(
      "description.json");

  QVERIFY(
    writer.write(
      path,
      description));

  QFile jsonFile(
    QString::fromStdString(path));

  QVERIFY(
    jsonFile.open(
      QIODevice::ReadOnly));

  const QByteArray data =
    jsonFile.readAll();

  QVERIFY(
    !data.isEmpty());

  QJsonParseError error;

  const QJsonDocument document =
    QJsonDocument::fromJson(
      data,
      &error);

  QCOMPARE(
    error.error,
    QJsonParseError::NoError);

  QVERIFY(
    document.isObject());

  const QJsonObject root =
    document.object();

  QCOMPARE(
    root["version"].toInt(),
    1);

  const QJsonArray files =
    root["files"].toArray();

  QCOMPARE(
    files.size(),
    3);

  for (std::size_t n = 0; n < jsonFiles.size(); ++n)
  {
    const auto &jsonFile = jsonFiles[n];

    const QJsonObject jsonFileDescription = files[n].toObject();

    QCOMPARE(
      jsonFileDescription["name"].toString(),
      jsonFile.name);

    QCOMPARE(
      jsonFileDescription["frequency"].toInt(),
      jsonFile.frequency);

    QCOMPARE(
      jsonFileDescription["dataType"].toString(),
      jsonFile.dataType);

    const QJsonArray signalIds =
      jsonFileDescription["signals"].toArray();

    QCOMPARE(
      signalIds.size(),
      jsonFile.signalIds.size());

    for (std::size_t i = 0; i < jsonFile.signalIds.size(); ++i)
    {
      const QJsonObject jsonSignal =
        signalIds[i].toObject();

      auto signal = jsonFile.signalIds[i];

      QCOMPARE(
        jsonSignal["index"].toInt(),
        signal.index);

      QCOMPARE(
        jsonSignal["id"].toInt(),
        signal.id.value);

      QCOMPARE(
        jsonSignal["name"].toString(),
        signal.name);

      QCOMPARE(
        jsonSignal["kind"].toString(),
        signal.kind == SignalKind::Raw ? "Raw" : "Calculated");

      if (signal.kind == SignalKind::Raw)
      {
        QVERIFY(signal.module.has_value());
        QVERIFY(signal.channel.has_value());

        QVERIFY(
          jsonSignal.contains("module"));

        QCOMPARE(
          jsonSignal["module"].toInt(),
          signal.module->value);

        QVERIFY(
          jsonSignal.contains("channel"));

        QCOMPARE(
          jsonSignal["channel"].toInt(),
          signal.channel->value);
      } else
      {
        QVERIFY(
          !jsonSignal.contains("module"));

        QVERIFY(
          !jsonSignal.contains("channel"));
      }
    }
  }
}

void tst_dataarchive::test_archiveDescription_archiveDescriptionBuilder()
{
  using namespace qds;
  auto cfg = createTestConfig_calculate(ModuleType::Test);

  ArchiveDescriptionBuilder builder;
  ArchiveDescription description;

  QVERIFY(builder.build(cfg, description));

  ArchiveDescriptionWriter writer;

  const auto path =
    getFilePath(
      "description.json");

  QVERIFY(
    writer.write(
      path,
      description));

  QFile jsonFile(
    QString::fromStdString(path));

  QVERIFY(
    jsonFile.open(
      QIODevice::ReadOnly));

  const QByteArray data =
    jsonFile.readAll();

  QVERIFY(
    !data.isEmpty());

  QJsonParseError error;

  const QJsonDocument document =
    QJsonDocument::fromJson(
      data,
      &error);

  QCOMPARE(
    error.error,
    QJsonParseError::NoError);

  QVERIFY(
    document.isObject());

  const QJsonObject root =
    document.object();

  QCOMPARE(
    root["version"].toInt(),
    1);

  const QJsonArray files =
    root["files"].toArray();

  QCOMPARE(
    files.size(),
    4);

  // 1 файл raw_1000Hz.dat

  QJsonObject jsonFileDescription =
    files[0].toObject();

  QCOMPARE(
    jsonFileDescription["name"].toString(),
    "raw_1000Hz.dat");

  QCOMPARE(
    jsonFileDescription["frequency"].toInt(),
    1000);

  QCOMPARE(
    jsonFileDescription["dataType"].toString(),
    "float");

  QJsonArray signalIds =
    jsonFileDescription["signals"].toArray();

  QCOMPARE(
    signalIds.size(),
    1);

  QJsonObject signal =
    signalIds[0].toObject();

  QCOMPARE(
    signal["index"].toInt(),
    0);

  QCOMPARE(
    signal["id"].toInt(),
    0);

  QCOMPARE(
    signal["name"].toString(),
    "Raw0");

  QCOMPARE(
    signal["kind"].toString(),
    "Raw");

  QCOMPARE(
    signal.contains("module"), true);

  QCOMPARE(
    signal.contains("channel"), true);

  QCOMPARE(
    signal["module"].toInt(),
    0);

  QCOMPARE(
    signal["channel"].toInt(),
    0);

  // 2 файл raw_100Hz.dat

  jsonFileDescription =
    files[1].toObject();

  QCOMPARE(
    jsonFileDescription["name"].toString(),
    "raw_100Hz.dat");

  QCOMPARE(
    jsonFileDescription["frequency"].toInt(),
    100);

  QCOMPARE(
    jsonFileDescription["dataType"].toString(),
    "float");

  signalIds =
    jsonFileDescription["signals"].toArray();

  QCOMPARE(
    signalIds.size(),
    1);

  signal =
    signalIds[0].toObject();

  QCOMPARE(
    signal["index"].toInt(),
    0);

  QCOMPARE(
    signal["id"].toInt(),
    1);

  QCOMPARE(
    signal["name"].toString(),
    "Raw1");

  QCOMPARE(
    signal["kind"].toString(),
    "Raw");

  QCOMPARE(
    signal.contains("module"), true);

  QCOMPARE(
    signal.contains("channel"), true);

  QCOMPARE(
    signal["module"].toInt(),
    0);

  QCOMPARE(
    signal["channel"].toInt(),
    1);

  // 3 calculated_100Hz.dat

  jsonFileDescription =
    files[2].toObject();

  QCOMPARE(
    jsonFileDescription["name"].toString(),
    "calculated_100Hz.dat");

  QCOMPARE(
    jsonFileDescription["frequency"].toInt(),
    100);

  QCOMPARE(
    jsonFileDescription["dataType"].toString(),
    "float");

  signalIds =
    jsonFileDescription["signals"].toArray();

  QCOMPARE(
    signalIds.size(),
    1);

  signal =
    signalIds[0].toObject();

  QCOMPARE(
    signal["index"].toInt(),
    0);

  QCOMPARE(
    signal["id"].toInt(),
    17);

  QCOMPARE(
    signal["name"].toString(),
    "A");

  QCOMPARE(
    signal["kind"].toString(),
    "Calculated");

  QCOMPARE(
    signal.contains("module"), false);

  QCOMPARE(
    signal.contains("channel"), false);

  // 4 calculated_10Hz.dat

  jsonFileDescription =
    files[3].toObject();

  QCOMPARE(
    jsonFileDescription["name"].toString(),
    "calculated_10Hz.dat");

  QCOMPARE(
    jsonFileDescription["frequency"].toInt(),
    10);

  QCOMPARE(
    jsonFileDescription["dataType"].toString(),
    "float");

  signalIds =
    jsonFileDescription["signals"].toArray();

  QCOMPARE(
    signalIds.size(),
    2);

  signal =
    signalIds[0].toObject();

  QCOMPARE(
    signal["index"].toInt(),
    0);

  QCOMPARE(
    signal["id"].toInt(),
    4);

  QCOMPARE(
    signal["name"].toString(),
    "B");

  QCOMPARE(
    signal["kind"].toString(),
    "Calculated");

  QCOMPARE(
    signal.contains("module"), false);

  QCOMPARE(
    signal.contains("channel"), false);

  signal =
    signalIds[1].toObject();

  QCOMPARE(
    signal["index"].toInt(),
    1);

  QCOMPARE(
    signal["id"].toInt(),
    23);

  QCOMPARE(
    signal["name"].toString(),
    "C");

  QCOMPARE(
    signal["kind"].toString(),
    "Calculated");

  QCOMPARE(
    signal.contains("module"), false);

  QCOMPARE(
    signal.contains("channel"), false);
}

void tst_dataarchive::test_DataBlockQueue_deque()
{
  using namespace qds;

  FakeLCardModule module(3, 3);

  RawMemory raw;
  raw.initialize(9);

  QVERIFY(module.start());

  DataBlockQueue queue;

  //
  // Block 1
  //
  QCOMPARE(
    module.readBlock(raw.values()),
    std::size_t{3});

  queue.push(
    ModuleId{10},
    100,
    raw.values(),
    3,
    3,
    1000);

  //
  // Block 2
  //
  QCOMPARE(
    module.readBlock(raw.values()),
    std::size_t{3});

  queue.push(
    ModuleId{20},
    101,
    raw.values(),
    3,
    3,
    1000);

  module.stop();

  //
  // Pop block 1
  //
  DataStreamEvent event;
  const auto& block =
    std::get<DataBlock>(event);

  QVERIFY(queue.pop(event));

  QCOMPARE(block.module, ModuleId{10});
  QCOMPARE(block.firstFrameIndex, 100);
  QCOMPARE(block.channelCount, std::size_t{3});
  QCOMPARE(block.frameCount, std::size_t{3});
  QCOMPARE(block.values.size(), std::size_t{9});
  QCOMPARE(block.frameRate, 1000.0);

  QCOMPARE(block.values[0], 0.0);
  QCOMPARE(block.values[1], 1.0);
  QCOMPARE(block.values[2], 2.0);
  QCOMPARE(block.values[3], 3.0);
  QCOMPARE(block.values[4], 4.0);
  QCOMPARE(block.values[5], 5.0);
  QCOMPARE(block.values[6], 6.0);
  QCOMPARE(block.values[7], 7.0);
  QCOMPARE(block.values[8], 8.0);

  //
  // Pop block 2
  //
  QVERIFY(queue.pop(event));

  QCOMPARE(block.module, ModuleId{20});
  QCOMPARE(block.firstFrameIndex, 101);
  QCOMPARE(block.channelCount, std::size_t{3});
  QCOMPARE(block.frameCount, std::size_t{3});
  QCOMPARE(block.values.size(), std::size_t{9});
  QCOMPARE(block.frameRate, 1000.0);

  QCOMPARE(block.values[0], 9.0);
  QCOMPARE(block.values[1], 10.0);
  QCOMPARE(block.values[2], 11.0);
  QCOMPARE(block.values[3], 12.0);
  QCOMPARE(block.values[4], 13.0);
  QCOMPARE(block.values[5], 14.0);
  QCOMPARE(block.values[6], 15.0);
  QCOMPARE(block.values[7], 16.0);
  QCOMPARE(block.values[8], 17.0);

  //
  // Queue empty
  //
  QVERIFY(!queue.pop(event));
}

void tst_dataarchive::test_DataBlockQueue_waitPop()
{
  using namespace qds;

  FakeLCardModule module(3, 3);

  RawMemory raw;
  raw.initialize(9);

  QVERIFY(module.start());

  DataBlockQueue queue;

  DataStreamEvent event;
  const auto& block =
    std::get<DataBlock>(event);

  bool result = false;

  std::thread thread(
    [&]() {
      result = queue.waitPop(event);
    });

  QCOMPARE(
    module.readBlock(raw.values()),
    std::size_t{3});

  queue.push(
    ModuleId{777},
    1,
    raw.values(),
    3,
    3,
    1000);

  thread.join();

  QVERIFY(result);

  QCOMPARE(block.module, ModuleId{777});
  QCOMPARE(block.firstFrameIndex, 1);
  QCOMPARE(block.channelCount, std::size_t{3});
  QCOMPARE(block.frameCount, std::size_t{3});
  QCOMPARE(block.values.size(), std::size_t{9});
  QCOMPARE(block.frameRate, 1000.0);

  QCOMPARE(block.values[0], 0.0);
  QCOMPARE(block.values[1], 1.0);
  QCOMPARE(block.values[2], 2.0);
  QCOMPARE(block.values[3], 3.0);
  QCOMPARE(block.values[4], 4.0);
  QCOMPARE(block.values[5], 5.0);
  QCOMPARE(block.values[6], 6.0);
  QCOMPARE(block.values[7], 7.0);
  QCOMPARE(block.values[8], 8.0);
}

void tst_dataarchive::test_DataBlockQueue_stop()
{
  using namespace qds;

  DataStreamEvent event;
  const auto& block =
    std::get<DataBlock>(event);

  DataBlockQueue queue;

  bool result = true;

  std::thread thread(
    [&]() {
      result = queue.waitPop(event);
    });

  queue.stop();

  thread.join();

  QVERIFY(!result);

  QCOMPARE(block.module, ModuleId{0});
  QCOMPARE(block.firstFrameIndex, 0);
  QCOMPARE(block.channelCount, std::size_t{0});
  QCOMPARE(block.frameCount, std::size_t{0});
  QCOMPARE(block.values.size(), std::size_t{0});
}

void tst_dataarchive::test_DataBlockQueue_stop_drains_queue()
{
  using namespace qds;

  DataBlockQueue queue;

  const std::vector<double> values1{
    1.0, 2.0, 3.0
  };

  const std::vector<double> values2{
    4.0, 5.0, 6.0
  };

  queue.push(
    ModuleId{10},
    777,
    values1,
    3,
    1,
    1000);

  queue.push(
    ModuleId{20},
    777,
    values2,
    3,
    1,
    1000);

  queue.stop();

  DataStreamEvent event;
  const auto& block =
    std::get<DataBlock>(event);

  QVERIFY(queue.waitPop(event));
  QCOMPARE(block.module, ModuleId{10});
  QCOMPARE(block.firstFrameIndex, 777);
  QCOMPARE(block.values[0], 1.0);
  QCOMPARE(block.frameRate, 1000.0);

  QVERIFY(queue.waitPop(event));
  QCOMPARE(block.module, ModuleId{20});
  QCOMPARE(block.firstFrameIndex, 777);
  QCOMPARE(block.values[0], 4.0);
  QCOMPARE(block.frameRate, 1000.0);

  QVERIFY(!queue.waitPop(event));
}

void tst_dataarchive::test_DataBlockQueue_firstFrameIndex()
{
  using namespace qds;

  auto module = std::make_unique<FakeLCardModule>(3, 3);

  DataBlockQueue queue;
  FakeClock clock;

  LCardDataSource source(
    ModuleId{0},
    3,
    std::move(module),
    clock,
    &queue);

  RawMemory raw;
  raw.initialize(3);

  DataStreamEvent event;
  const auto& block =
    std::get<DataBlock>(event);


  QVERIFY(source.start());

  QTest::qWait(10);
  source.stop();

  QVERIFY(queue.waitPop(event));

  QCOMPARE(block.module, ModuleId{0});
  QCOMPARE(block.firstFrameIndex, 0);
  QCOMPARE(block.channelCount, std::size_t{3});
  QCOMPARE(block.frameCount, std::size_t{3});
  QCOMPARE(block.values.size(), std::size_t{9});
  QCOMPARE(block.frameRate, 1000.0);

  QCOMPARE(block.values[0], 0.0);
  QCOMPARE(block.values[1], 1.0);
  QCOMPARE(block.values[2], 2.0);
  QCOMPARE(block.values[3], 3.0);
  QCOMPARE(block.values[4], 4.0);
  QCOMPARE(block.values[5], 5.0);
  QCOMPARE(block.values[6], 6.0);
  QCOMPARE(block.values[7], 7.0);
  QCOMPARE(block.values[8], 8.0);

  QVERIFY(queue.waitPop(event));

  QCOMPARE(block.module, ModuleId{0});
  QCOMPARE(block.firstFrameIndex, 3);
  QCOMPARE(block.channelCount, std::size_t{3});
  QCOMPARE(block.frameCount, std::size_t{3});
  QCOMPARE(block.values.size(), std::size_t{9});
  QCOMPARE(block.frameRate, 1000.0);

  QCOMPARE(block.values[0], 9.0);
  QCOMPARE(block.values[1], 10.0);
  QCOMPARE(block.values[2], 11.0);
  QCOMPARE(block.values[3], 12.0);
  QCOMPARE(block.values[4], 13.0);
  QCOMPARE(block.values[5], 14.0);
  QCOMPARE(block.values[6], 15.0);
  QCOMPARE(block.values[7], 16.0);
  QCOMPARE(block.values[8], 17.0);

  auto index = block.firstFrameIndex / 3;
  while (queue.size() > 0 && queue.waitPop(event))
  {
    QCOMPARE(block.module, ModuleId{0});
    QCOMPARE(block.firstFrameIndex, ++index * 3);
    QCOMPARE(block.channelCount, std::size_t{3});
    QCOMPARE(block.frameCount, std::size_t{3});
    QCOMPARE(block.values.size(), std::size_t{9});
    QCOMPARE(block.frameRate, 1000.0);

    QCOMPARE(block.values[0], static_cast<double>(index * 9.0));
  }

  auto lastBlock = block;

  QVERIFY(source.start());

  QTest::qWait(10);
  source.stop();

  QVERIFY(queue.waitPop(event));

  QCOMPARE(block.module, ModuleId{0});
  QCOMPARE(block.firstFrameIndex, lastBlock.firstFrameIndex + 3);
  QCOMPARE(block.channelCount, std::size_t{3});
  QCOMPARE(block.frameCount, std::size_t{3});
  QCOMPARE(block.values.size(), std::size_t{9});
  QCOMPARE(block.frameRate, 1000.0);

  QCOMPARE(block.values[0], static_cast<double>(block.firstFrameIndex * (9.0 / 3)));

  source.stop();
}

void tst_dataarchive::test_DataBlockQueue_firstFrameIndex_two_blocks()
{
  using namespace qds;

  auto module = std::make_unique<FakeLCardModule>(3, 3);

  DataBlockQueue queue;
  FakeClock clock;

  LCardDataSource source(
    ModuleId{0},
    3,
    std::move(module),
    clock,
    &queue);

  RawMemory raw;
  raw.initialize(3);

  DataBlock block;

  QVERIFY(source.start());

  QTest::qWait(1);

  source.stop();

  DataStreamEvent event1;
  const auto& block1 =
    std::get<DataBlock>(event1);

  DataStreamEvent event2;
  const auto& block2 =
    std::get<DataBlock>(event2);

  QVERIFY(queue.pop(event1));
  QVERIFY(queue.pop(event2));

  QCOMPARE(block1.firstFrameIndex, uint64_t{0});
  QCOMPARE(block1.frameCount, std::size_t{3});

  QCOMPARE(
    block2.firstFrameIndex,
    block1.firstFrameIndex +
      block1.frameCount);

}

void tst_dataarchive::test_DataBlockQueue_firstFrameIndex_three_blocks()
{
  using namespace qds;

  auto module = std::make_unique<SmartBlockLCardModule>(3, 3, 3);
  auto* fake = module.get();

  DataBlockQueue queue;
  FakeClock clock;

  LCardDataSource source(
    ModuleId{0},
    3,
    std::move(module),
    clock,
    &queue);

  RawMemory raw;
  raw.initialize(3);


  QVERIFY(source.start());
  QTest::qWait(5);

  source.stop();

  DataStreamEvent event;
  const auto& block =
    std::get<DataBlock>(event);

  DataStreamEvent event1;
  const auto& block1 =
    std::get<DataBlock>(event1);

  DataStreamEvent event2;
  const auto& block2 =
    std::get<DataBlock>(event2);

  DataStreamEvent event3;
  const auto& block3 =
    std::get<DataBlock>(event3);

  QVERIFY(queue.pop(event1));
  QVERIFY(queue.pop(event2));
  QVERIFY(queue.pop(event3));

  QVERIFY(!queue.pop(event));

  QCOMPARE(block1.firstFrameIndex, uint64_t{0});
  QCOMPARE(block1.frameCount, std::size_t{3});

  QCOMPARE(block2.firstFrameIndex, block1.firstFrameIndex + block1.frameCount);
  QCOMPARE(block3.firstFrameIndex, block2.firstFrameIndex + block2.frameCount);

  fake->setCount(3);

  QVERIFY(source.start());
  QTest::qWait(5);

  source.stop();

  QVERIFY(queue.pop(event1));
  QVERIFY(queue.pop(event2));
  QVERIFY(queue.pop(event3));

  QVERIFY(!queue.pop(event));

  QCOMPARE(block1.firstFrameIndex, uint64_t{9});
  QCOMPARE(block1.frameCount, std::size_t{3});

  QCOMPARE(block2.firstFrameIndex, block1.firstFrameIndex + block1.frameCount);
  QCOMPARE(block3.firstFrameIndex, block2.firstFrameIndex + block2.frameCount);
}

void tst_dataarchive::test_DataStreamEventSink_base()
{
  using namespace qds;

  FakeClock clock;
  clock.setTimestamp(123456);
  clock.setWallClockTime(987654);

  auto module = std::make_unique<SmartBlockLCardModule>(3, 3, 3);

  FakeDataStreamEventSink sink;

  DataBlockQueue queue;

  LCardDataSource source(
    ModuleId{777},
    3,
    std::move(module),
    clock,
    &queue,
    &sink);

  QVERIFY(source.start());
  QTest::qWait(5);

  source.stop();

  auto const &anchor = sink.m_anchor;

  QCOMPARE(anchor.module, ModuleId{777});
  QCOMPARE(anchor.firstFrameIndex, 0);
  QCOMPARE(anchor.startTimestamp, Timestamp{123456});
  QCOMPARE(anchor.startWallTime, WallClockTime{987654});
  QCOMPARE(anchor.frameRate, 1000);

  clock.advance(500000);

  QVERIFY(source.start());
  QTest::qWait(5);

  source.stop();

  QCOMPARE(anchor.module, ModuleId{777});
  QCOMPARE(anchor.firstFrameIndex, uint64_t{9});
  QCOMPARE(anchor.startTimestamp, Timestamp{623456});
  QCOMPARE(anchor.startWallTime, WallClockTime{1487654});
  QCOMPARE(anchor.frameRate, 1000.0);
}

void tst_dataarchive::test_DataStreamEvent_base()
{
  using namespace qds;

  DataStreamEvent event;

  QVERIFY(
    std::holds_alternative<DataStreamAnchor>(
      event));
}

void tst_dataarchive::test_DataBlockQueue_anchor_only()
{
  using namespace qds;

  DataBlockQueue queue;

  DataStreamAnchor anchor;
  anchor.module = ModuleId{10};
  anchor.firstFrameIndex = 0;
  anchor.startTimestamp = Timestamp{1000};
  anchor.startWallTime = WallClockTime{2000};
  anchor.frameRate = 1000.0;

  queue.startStream(anchor);

  DataStreamEvent event;

  QVERIFY(queue.pop(event));

  QVERIFY(
    std::holds_alternative<DataStreamAnchor>(
      event));

  const auto& result =
    std::get<DataStreamAnchor>(event);

  QCOMPARE(result.module, ModuleId{10});
  QCOMPARE(result.firstFrameIndex, uint64_t{0});
  QCOMPARE(result.startTimestamp, Timestamp{1000});
  QCOMPARE(result.startWallTime, WallClockTime{2000});
  QCOMPARE(result.frameRate, 1000.0);

  QVERIFY(!queue.pop(event));
}

void tst_dataarchive::test_DataBlockQueue_anchor_block()
{
  using namespace qds;

  DataBlockQueue queue;

  DataStreamAnchor anchor;
  anchor.module = ModuleId{10};
  anchor.firstFrameIndex = 0;
  anchor.startTimestamp = Timestamp{1000};
  anchor.startWallTime = WallClockTime{2000};
  anchor.frameRate = 1000.0;

  queue.startStream(anchor);

  const std::vector<double> values{
    0.0, 1.0, 2.0,
    3.0, 4.0, 5.0,
    6.0, 7.0, 8.0
  };

  queue.push(
    ModuleId{10},
    0,
    values,
    3,
    3,
    1000.0);

  DataStreamEvent event;

  QVERIFY(queue.pop(event));
  QVERIFY(
    std::holds_alternative<DataStreamAnchor>(
      event));

  QVERIFY(queue.pop(event));
  QVERIFY(
    std::holds_alternative<DataBlock>(
      event));

  const auto& block =
    std::get<DataBlock>(event);

  QCOMPARE(block.module, ModuleId{10});
  QCOMPARE(block.firstFrameIndex, uint64_t{0});
  QCOMPARE(block.channelCount, std::size_t{3});
  QCOMPARE(block.frameCount, std::size_t{3});
  QCOMPARE(block.values.size(), std::size_t{9});

  QVERIFY(!queue.pop(event));
}

void tst_dataarchive::test_DataBlockQueue_three_blocks()
{
  using namespace qds;

  DataBlockQueue queue;

  DataStreamAnchor anchor;
  anchor.module = ModuleId{777};
  anchor.firstFrameIndex = 0;
  anchor.frameRate = 1000.0;

  queue.startStream(anchor);

  const std::vector<double> values(9, 1.0);

  queue.push(
    ModuleId{777}, 0,
    values, 3, 3, 1000.0);

  queue.push(
    ModuleId{777}, 3,
    values, 3, 3, 1000.0);

  queue.push(
    ModuleId{777}, 6,
    values, 3, 3, 1000.0);

  QCOMPARE(queue.size(), std::size_t{4});

  DataStreamEvent event;

  QVERIFY(queue.pop(event));
  QVERIFY(
    std::holds_alternative<DataStreamAnchor>(
      event));

  for (uint64_t index :
       {uint64_t{0}, uint64_t{3}, uint64_t{6}})
  {
    QVERIFY(queue.pop(event));

    QVERIFY(
      std::holds_alternative<DataBlock>(
        event));

    const auto& block =
      std::get<DataBlock>(event);

    QCOMPARE(
      block.firstFrameIndex,
      index);
  }

  QVERIFY(!queue.pop(event));
}

void tst_dataarchive::test_LCardDataSource_stream_events()
{
  using namespace qds;

  FakeClock clock;
  clock.setTimestamp(123456);
  clock.setWallClockTime(987654);

  auto module =
    std::make_unique<SmartBlockLCardModule>(
      3,  // frames per block
      3,  // channels
      3); // blocks

  auto* fake = module.get();

  DataBlockQueue queue;

  LCardDataSource source(
    ModuleId{777},
    3,
    std::move(module),
    clock,
    &queue,
    &queue);

  QVERIFY(source.start());
  QTest::qWait(5);
  source.stop();

  DataStreamEvent event;

  // Anchor #1
  QVERIFY(queue.pop(event));
  QVERIFY(
    std::holds_alternative<DataStreamAnchor>(
      event));

  {
    const auto& anchor =
      std::get<DataStreamAnchor>(event);

    QCOMPARE(anchor.module, ModuleId{777});
    QCOMPARE(anchor.firstFrameIndex, uint64_t{0});
    QCOMPARE(anchor.startTimestamp, Timestamp{123456});
    QCOMPARE(anchor.startWallTime, WallClockTime{987654});
    QCOMPARE(anchor.frameRate, 1000.0);
  }

  // Block #1
  QVERIFY(queue.pop(event));
  QVERIFY(std::holds_alternative<DataBlock>(event));
  {
    const auto& block =
      std::get<DataBlock>(event);

    QCOMPARE(block.module, ModuleId{777});
    QCOMPARE(block.firstFrameIndex, uint64_t{0});
    QCOMPARE(block.channelCount, std::size_t{3});
    QCOMPARE(block.frameCount, std::size_t{3});
    QCOMPARE(block.frameRate, 1000.0);
  }

  // Block #2
  QVERIFY(queue.pop(event));
  QVERIFY(std::holds_alternative<DataBlock>(event));
  {
    const auto& block =
      std::get<DataBlock>(event);

    QCOMPARE(block.firstFrameIndex, uint64_t{3});
  }

  // Block #3
  QVERIFY(queue.pop(event));
  QVERIFY(std::holds_alternative<DataBlock>(event));

  {
    const auto& block =
      std::get<DataBlock>(event);

    QCOMPARE(block.firstFrameIndex, uint64_t{6});
  }

  QVERIFY(!queue.pop(event));

  // Новый acquisition segment.
  clock.advance(500000);
  fake->setCount(3);

  QVERIFY(source.start());
  QTest::qWait(5);
  source.stop();

  // Anchor #2
  QVERIFY(queue.pop(event));
  QVERIFY(
    std::holds_alternative<DataStreamAnchor>(
      event));

  {
    const auto& anchor =
      std::get<DataStreamAnchor>(event);

    QCOMPARE(anchor.module, ModuleId{777});
    QCOMPARE(anchor.firstFrameIndex, uint64_t{9});

    QCOMPARE(
      anchor.startTimestamp,
      Timestamp{623456});

    QCOMPARE(
      anchor.startWallTime,
      WallClockTime{1487654});

    QCOMPARE(anchor.frameRate, 1000.0);
  }

  // Первый block нового segment.
  QVERIFY(queue.pop(event));
  QVERIFY(std::holds_alternative<DataBlock>(event));
  {
    const auto& block =
      std::get<DataBlock>(event);

    QCOMPARE(block.firstFrameIndex, uint64_t{9});
  }
}

void tst_dataarchive::test_DataBlockQueue_stream_events()
{
  using namespace qds;

  DataBlockQueue queue;

  DataStreamAnchor anchor;
  anchor.module = ModuleId{10};
  anchor.firstFrameIndex = 0;
  anchor.startTimestamp = Timestamp{1000};
  anchor.startWallTime = WallClockTime{2000};
  anchor.frameRate = 1000.0;

  queue.startStream(anchor);

  const std::vector<double> values{
    0.0, 1.0, 2.0,
    3.0, 4.0, 5.0,
    6.0, 7.0, 8.0
  };

  queue.push(
    ModuleId{10},
    0,
    values,
    3,
    3,
    1000.0);

  DataStreamEvent event;

  QVERIFY(queue.pop(event));

  QVERIFY(
    std::holds_alternative<DataStreamAnchor>(
      event));

  const auto& resultAnchor =
    std::get<DataStreamAnchor>(event);

  QCOMPARE(
    resultAnchor.module,
    ModuleId{10});

  QCOMPARE(
    resultAnchor.firstFrameIndex,
    uint64_t{0});

  QCOMPARE(
    resultAnchor.startTimestamp,
    Timestamp{1000});

  QVERIFY(queue.pop(event));

  QVERIFY(
    std::holds_alternative<DataBlock>(
      event));

  const auto& block =
    std::get<DataBlock>(event);

  QCOMPARE(block.module, ModuleId{10});
  QCOMPARE(block.firstFrameIndex, uint64_t{0});
  QCOMPARE(block.frameCount, std::size_t{3});

  QVERIFY(!queue.pop(event));
}

void tst_dataarchive::test_DataStreamTime_base()
{
  using namespace qds;

  DataStreamTime time;

  DataStreamAnchor anchor;
  anchor.module = ModuleId{10};
  anchor.firstFrameIndex = 100;
  anchor.startTimestamp = Timestamp{1'000'000};
  anchor.startWallTime = WallClockTime{10'000'000};
  anchor.frameRate = 1000.0;

  time.startStream(anchor);

  Timestamp timestamp;
  WallClockTime wallTime;

  QVERIFY(
    time.timestamp(
      ModuleId{10},
      100,
      timestamp,
      wallTime));

  QCOMPARE(
    timestamp,
    Timestamp{1'000'000});

  QCOMPARE(
    wallTime,
    WallClockTime{10'000'000});

  QVERIFY(
    time.timestamp(
      ModuleId{10},
      103,
      timestamp,
      wallTime));

  QCOMPARE(
    timestamp,
    Timestamp{1'003'000});

  QCOMPARE(
    wallTime,
    WallClockTime{10'003'000});


  QVERIFY(
    !time.timestamp(
      ModuleId{11},
      100,
      timestamp,
      wallTime));

  QVERIFY(
    !time.timestamp(
      ModuleId{10},
      99,
      timestamp,
      wallTime));
}

void tst_dataarchive::test_DataStreamTime_frameRate_0()
{
  using namespace qds;

  DataStreamTime time;

  DataStreamAnchor anchor;
  anchor.module = ModuleId{10};
  anchor.firstFrameIndex = 100;
  anchor.startTimestamp = Timestamp{1'000'000};
  anchor.startWallTime = WallClockTime{10'000'000};
  anchor.frameRate = 0;

  time.startStream(anchor);

  Timestamp timestamp;
  WallClockTime wallTime;

  QVERIFY(
    !time.timestamp(
      ModuleId{10},
      100,
      timestamp,
      wallTime));

  QCOMPARE(
    timestamp,
    Timestamp{0});

  QCOMPARE(
    wallTime,
    WallClockTime{0});

}

void tst_dataarchive::test_DataStreamTime_restart()
{
  using namespace qds;

  DataStreamTime time;

  DataStreamAnchor anchor1;
  anchor1.module = ModuleId{10};
  anchor1.firstFrameIndex = 0;
  anchor1.startTimestamp = Timestamp{1'000'000};
  anchor1.startWallTime =
    WallClockTime{10'000'000};
  anchor1.frameRate = 1000.0;

  time.startStream(anchor1);

  Timestamp timestamp;
  WallClockTime wallTime;

  QVERIFY(
    time.timestamp(
      ModuleId{10},
      9,
      timestamp,
      wallTime));

  QCOMPARE(
    timestamp,
    Timestamp{1'009'000});

  QCOMPARE(
    wallTime,
    WallClockTime{10'009'000});

  // Новый acquisition segment.
  DataStreamAnchor anchor2;
  anchor2.module = ModuleId{10};
  anchor2.firstFrameIndex = 10;
  anchor2.startTimestamp = Timestamp{2'000'000};
  anchor2.startWallTime =
    WallClockTime{20'000'000};
  anchor2.frameRate = 1000.0;

  time.startStream(anchor2);

  QVERIFY(
    !time.timestamp(
      ModuleId{10},
      9,
      timestamp,
      wallTime));

  QVERIFY(
    time.timestamp(
      ModuleId{10},
      10,
      timestamp,
      wallTime));

  QCOMPARE(
    timestamp,
    Timestamp{2'000'000});

  QCOMPARE(
    wallTime,
    WallClockTime{20'000'000});

  QVERIFY(
    time.timestamp(
      ModuleId{10},
      13,
      timestamp,
      wallTime));

  QCOMPARE(
    timestamp,
    Timestamp{2'003'000});

  QCOMPARE(
    wallTime,
    WallClockTime{20'003'000});
}

void tst_dataarchive::test_DataStreamTime_frameRate_44100()
{
  using namespace qds;

  DataStreamTime time;

  DataStreamAnchor anchor;
  anchor.module = ModuleId{10};
  anchor.firstFrameIndex = 0;
  anchor.startTimestamp = Timestamp{0};
  anchor.startWallTime = WallClockTime{0};
  anchor.frameRate = 44100.0;

  time.startStream(anchor);


  Timestamp timestamp;
  WallClockTime wallTime;

  QVERIFY(
    time.timestamp(
      ModuleId{10},
      1,
      timestamp,
      wallTime));

  QCOMPARE(timestamp, Timestamp{23});

  QVERIFY(
    time.timestamp(
      ModuleId{10},
      7,
      timestamp,
      wallTime));

  QCOMPARE(timestamp, Timestamp{159});

  QVERIFY(
    time.timestamp(
      ModuleId{10},
      10,
      timestamp,
      wallTime));

  QCOMPARE(timestamp, Timestamp{227});

  QVERIFY(
    time.timestamp(
      ModuleId{10},
      70,
      timestamp,
      wallTime));

  QCOMPARE(timestamp, Timestamp{1587});

}

void tst_dataarchive::
  test_DataStreamReader_block()
{
  using namespace qds;

  DataStreamReader reader;

  DataStreamAnchor anchor;
  anchor.module = ModuleId{10};
  anchor.firstFrameIndex = 100;
  anchor.startTimestamp = Timestamp{1'000'000};
  anchor.startWallTime =
    WallClockTime{10'000'000};
  anchor.frameRate = 1000.0;

  DataStreamEvent event = anchor;

  std::vector<DataStreamFrame> frames;

  QVERIFY(reader.process(event, frames));
  QVERIFY(frames.empty());

  DataBlock block;
  block.module = ModuleId{10};
  block.firstFrameIndex = 100;
  block.frameRate = 1000.0;
  block.channelCount = 3;
  block.frameCount = 3;

  block.values = {
    10, 11, 12,
    20, 21, 22,
    30, 31, 32
  };

  event = block;

  QVERIFY(reader.process(event, frames));

  QCOMPARE(frames.size(), std::size_t{3});

  QCOMPARE(frames[0].module, ModuleId{10});
  QCOMPARE(frames[0].frameIndex, uint64_t{100});
  QCOMPARE(frames[0].timestamp, Timestamp{1'000'000});
  QCOMPARE(frames[0].wallTime, WallClockTime{10'000'000});

  QCOMPARE(frames[1].frameIndex, uint64_t{101});
  QCOMPARE(frames[1].timestamp, Timestamp{1'001'000});

  QCOMPARE(frames[2].frameIndex, uint64_t{102});
  QCOMPARE(frames[2].timestamp, Timestamp{1'002'000});

  QCOMPARE(
    frames[0].values,
    std::vector<double>({10, 11, 12}));

  QCOMPARE(
    frames[1].values,
    std::vector<double>({20, 21, 22}));

  QCOMPARE(
    frames[2].values,
    std::vector<double>({30, 31, 32}));
}

void tst_dataarchive::test_DataStreamReader_block_before_anchor()
{
  using namespace qds;

  DataStreamReader reader;

  DataBlock block;
  block.module = ModuleId{10};
  block.firstFrameIndex = 100;
  block.frameRate = 1000.0;
  block.channelCount = 3;
  block.frameCount = 3;

  block.values = {
    10, 11, 12,
    20, 21, 22,
    30, 31, 32
  };

  DataStreamEvent event = block;

  std::vector<DataStreamFrame> frames;

  QVERIFY(
    !reader.process(
      event,
      frames));

  QVERIFY(frames.empty());
}

void tst_dataarchive::test_DataStreamReader_bad_block()
{
  using namespace qds;

  DataStreamReader reader;

  DataStreamAnchor anchor;
  anchor.module = ModuleId{10};
  anchor.firstFrameIndex = 100;
  anchor.startTimestamp = Timestamp{1'000'000};
  anchor.startWallTime = WallClockTime{10'000'000};
  anchor.frameRate = 1000.0;

  DataStreamEvent event = anchor;

  std::vector<DataStreamFrame> frames;

  QVERIFY(reader.process(event, frames));

  DataBlock block;
  block.module = ModuleId{10};
  block.firstFrameIndex = 100;
  block.frameRate = 1000.0;
  block.channelCount = 3;
  block.frameCount = 3;

  block.values = {
    10, 11, 12,
    20, 21, 22,
    30, 31
  };

  event = block;

  QVERIFY(!reader.process(event, frames));
  QVERIFY(frames.empty());
}

void tst_dataarchive::test_FrameAssembler_wait_for_all_modules()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FrameAssembler assembler(
    cfg,
    layout);

  DataStreamFrame a0;
  a0.module = ModuleId{0};
  a0.frameIndex = 0;
  a0.timestamp = Timestamp{1'000};
  a0.wallTime = WallClockTime{10'000};
  a0.values = {10, 11};

  auto result =
    assembler.push(a0);

  QVERIFY(!result.has_value());

  DataStreamFrame a1;
  a1.module = ModuleId{0};
  a1.frameIndex = 1;
  a1.timestamp = Timestamp{2'000};
  a1.wallTime = WallClockTime{11'000};
  a1.values = {20, 21};

  result = assembler.push(a1);

  QVERIFY(!result.has_value());

  DataStreamFrame b0;
  b0.module = ModuleId{1};
  b0.frameIndex = 0;
  b0.timestamp = Timestamp{2'500};
  b0.wallTime = WallClockTime{11'500};
  b0.values = {30, 31, 32};

  result = assembler.push(b0);

  QVERIFY(!result.has_value());

  DataStreamFrame c0;
  c0.module = ModuleId{2};
  c0.frameIndex = 0;
  c0.timestamp = Timestamp{3'000};
  c0.wallTime = WallClockTime{12'000};
  c0.values = {40, 41};

  result = assembler.push(c0);

  QVERIFY(result.has_value());

  QCOMPARE(
    result->timestamp,
    Timestamp{3'000});

  QCOMPARE(
    result->wallTime,
    WallClockTime{12'000});

  QCOMPARE(
    result->number,
    FrameNumber{0});

  QCOMPARE(result->raw().value(0), 20.0);
  QCOMPARE(result->raw().value(1), 21.0);

  QCOMPARE(result->raw().value(2), 30.0);
  QCOMPARE(result->raw().value(3), 31.0);
  QCOMPARE(result->raw().value(4), 32.0);

  QCOMPARE(result->raw().value(5), 40.0);
  QCOMPARE(result->raw().value(6), 41.0);


  DataStreamFrame d0;
  d0.module = ModuleId{30};
  d0.frameIndex = 0;
  d0.timestamp = Timestamp{3'500};
  d0.wallTime = WallClockTime{13'000};
  d0.values = {50, 49, 48};

  result = assembler.push(d0);

  QVERIFY(!result.has_value());

  DataStreamFrame a2;
  a2.module = ModuleId{0};
  a2.frameIndex = 2;
  a2.timestamp = Timestamp{4'000};
  a2.wallTime = WallClockTime{13'000};
  a2.values = {50, 51};

  result = assembler.push(a2);

  QVERIFY(result.has_value());

  QCOMPARE(
    result->number,
    FrameNumber{1});

  QCOMPARE(
    result->timestamp,
    Timestamp{4'000});

  QCOMPARE(result->raw().value(0), 50.0);
  QCOMPARE(result->raw().value(1), 51.0);

  // Последние значения остальных модулей
  QCOMPARE(result->raw().value(2), 30.0);
  QCOMPARE(result->raw().value(3), 31.0);
  QCOMPARE(result->raw().value(4), 32.0);
  QCOMPARE(result->raw().value(5), 40.0);
  QCOMPARE(result->raw().value(6), 41.0);

  DataStreamFrame c1;
  c1.module = ModuleId{2};
  c1.frameIndex = 3;
  c1.timestamp = Timestamp{5'000};
  c1.wallTime = WallClockTime{14'000};
  c1.values = {80, 81, 81};

  result = assembler.push(c1);

  QVERIFY(!result.has_value());


  DataStreamFrame b1;
  b1.module = ModuleId{1};
  b1.frameIndex = 1;
  b1.timestamp = Timestamp{6'000};
  b1.wallTime = WallClockTime{15'000};
  b1.values = {60, 61, 62};

  result = assembler.push(b1);

  QVERIFY(result.has_value());

  QCOMPARE(
    result->number,
    FrameNumber{2});

  QCOMPARE(
    result->timestamp,
    Timestamp{6'000});

  QCOMPARE(
    result->wallTime,
    WallClockTime{15'000});

  // Последний корректный module 0: a2
  QCOMPARE(result->raw().value(0), 50.0);
  QCOMPARE(result->raw().value(1), 51.0);

  // Новый module 1: b1
  QCOMPARE(result->raw().value(2), 60.0);
  QCOMPARE(result->raw().value(3), 61.0);
  QCOMPARE(result->raw().value(4), 62.0);

  // module 2 должен остаться от c0.
  // Ошибочный c1 сюда попасть не должен.
  QCOMPARE(result->raw().value(5), 40.0);
  QCOMPARE(result->raw().value(6), 41.0);

}

void tst_dataarchive::test_FrameAssembler_allow_incomplete()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FrameAssembler assembler(
    cfg,
    layout,
    FrameStartPolicy::AllowIncomplete);

  DataStreamFrame a0;
  a0.module = ModuleId{0};
  a0.values = {10, 11};
  a0.timestamp = Timestamp{1000};
  a0.wallTime = WallClockTime{10000};

  auto result = assembler.push(a0);

  QVERIFY(result.has_value());

  QCOMPARE(result->raw().value(0), 10.0);
  QCOMPARE(result->raw().value(1), 11.0);

  // module 1 ещё не получен
  QVERIFY(std::isnan(result->raw().value(2)));
  QVERIFY(std::isnan(result->raw().value(3)));
  QVERIFY(std::isnan(result->raw().value(4)));

  // module 2 ещё не получен
  QVERIFY(std::isnan(result->raw().value(5)));
  QVERIFY(std::isnan(result->raw().value(6)));

  DataStreamFrame b0;
  b0.module = ModuleId{1};
  b0.values = {20, 21, 22};
  b0.timestamp = Timestamp{2000};
  b0.wallTime = WallClockTime{11000};

  result = assembler.push(b0);

  QVERIFY(result.has_value());

  QCOMPARE(result->number, FrameNumber{1});
  QCOMPARE(result->timestamp, Timestamp{2000});

  // module 0 — последнее известное
  QCOMPARE(result->raw().value(0), 10.0);
  QCOMPARE(result->raw().value(1), 11.0);

  // module 1 — новые данные
  QCOMPARE(result->raw().value(2), 20.0);
  QCOMPARE(result->raw().value(3), 21.0);
  QCOMPARE(result->raw().value(4), 22.0);

  // module 2 всё ещё отсутствует
  QVERIFY(std::isnan(result->raw().value(5)));
  QVERIFY(std::isnan(result->raw().value(6)));

  DataStreamFrame c0;
  c0.module = ModuleId{2};
  c0.values = {30, 31};
  c0.timestamp = Timestamp{3000};
  c0.wallTime = WallClockTime{12000};

  result = assembler.push(c0);

  QVERIFY(result.has_value());

  QCOMPARE(result->number, FrameNumber{2});

  QCOMPARE(result->raw().value(0), 10.0);
  QCOMPARE(result->raw().value(1), 11.0);

  QCOMPARE(result->raw().value(2), 20.0);
  QCOMPARE(result->raw().value(3), 21.0);
  QCOMPARE(result->raw().value(4), 22.0);

  QCOMPARE(result->raw().value(5), 30.0);
  QCOMPARE(result->raw().value(6), 31.0);
}

void tst_dataarchive::test_DataStreamProcessor_wait_for_all()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  DataStreamReader reader;

  FrameAssembler assembler(
    cfg,
    layout,
    FrameStartPolicy::WaitForAllModules);

  BufferManager buffers;
  buffers.initialize(layout);

  FormulaAstRepository formulas;
  CalculationPlan plan;
  CalibrationRepository calibrations;

  SignalProcessor signalProcessor(
    layout,
    formulas,
    plan,
    calibrations);

  TestArchiveFrameWriter archive;

  // твой существующий TestLogger/FakeLogger
  TestLogger logger;

  DataStreamProcessor processor(
    reader,
    assembler,
    signalProcessor,
    buffers,
    archive,
    logger);


  DataStreamAnchor anchor0;
  anchor0.module = ModuleId{0};
  anchor0.firstFrameIndex = 0;
  anchor0.startTimestamp = Timestamp{1000};
  anchor0.startWallTime = WallClockTime{10000};
  anchor0.frameRate = 1000.0;

  QVERIFY(processor.process(anchor0));

  QCOMPARE(archive.frames.size(), std::size_t{0});
  QVERIFY(!buffers.ready());

  DataBlock block0;
  block0.module = ModuleId{0};
  block0.firstFrameIndex = 0;
  block0.frameRate = 1000.0;
  block0.channelCount = 2;
  block0.frameCount = 1;
  block0.values = {10, 11};

  QVERIFY(processor.process(block0));

  QCOMPARE(archive.frames.size(), std::size_t{0});
  QVERIFY(!buffers.ready());


  DataStreamAnchor anchor1;
  anchor1.module = ModuleId{1};
  anchor1.firstFrameIndex = 0;
  anchor1.startTimestamp = Timestamp{2000};
  anchor1.startWallTime = WallClockTime{11000};
  anchor1.frameRate = 1000.0;

  QVERIFY(processor.process(anchor1));

  DataBlock block1;
  block1.module = ModuleId{1};
  block1.firstFrameIndex = 0;
  block1.frameRate = 1000.0;
  block1.channelCount = 3;
  block1.frameCount = 1;
  block1.values = {20, 21, 22};

  QVERIFY(processor.process(block1));

  QCOMPARE(archive.frames.size(), std::size_t{0});
  QVERIFY(!buffers.ready());


  DataStreamAnchor anchor2;
  anchor2.module = ModuleId{2};
  anchor2.firstFrameIndex = 0;
  anchor2.startTimestamp = Timestamp{3000};
  anchor2.startWallTime = WallClockTime{12000};
  anchor2.frameRate = 1000.0;

  QVERIFY(processor.process(anchor2));

  DataBlock block2;
  block2.module = ModuleId{2};
  block2.firstFrameIndex = 0;
  block2.frameRate = 1000.0;
  block2.channelCount = 2;
  block2.frameCount = 1;
  block2.values = {30, 31};

  QVERIFY(processor.process(block2));


  QCOMPARE(
    archive.frames.size(),
    std::size_t{1});

  QVERIFY(buffers.ready());

  const Frame& archived =
    archive.frames[0];

  QCOMPARE(
    archived.number,
    FrameNumber{0});

  QCOMPARE(
    archived.timestamp,
    Timestamp{3000});

  QCOMPARE(
    archived.wallTime,
    WallClockTime{12000});

  QCOMPARE(archived.raw().value(0), 10.0);
  QCOMPARE(archived.raw().value(1), 11.0);

  QCOMPARE(archived.raw().value(2), 20.0);
  QCOMPARE(archived.raw().value(3), 21.0);
  QCOMPARE(archived.raw().value(4), 22.0);

  QCOMPARE(archived.raw().value(5), 30.0);
  QCOMPARE(archived.raw().value(6), 31.0);


  Frame latest;
  QVERIFY(buffers.readFrame(latest));

  QCOMPARE(
    latest.number,
    archived.number);

  QCOMPARE(
    latest.timestamp,
    archived.timestamp);

  QCOMPARE(
    latest.wallTime,
    archived.wallTime);

  QVERIFY(
    latest.raw().equals(
      archived.raw().values()));
}

void tst_dataarchive::test_DataStreamProcessor_livetime()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  DataStreamReader reader;

  FrameAssembler assembler(
    cfg,
    layout,
    FrameStartPolicy::AllowIncomplete);

  BufferManager buffers;
  buffers.initialize(layout);

  FormulaAstRepository formulas;
  CalculationPlan plan;
  CalibrationRepository calibrations;

  SignalProcessor signalProcessor(
    layout,
    formulas,
    plan,
    calibrations);

  TestArchiveFrameWriter archive;

  // твой существующий TestLogger/FakeLogger
  TestLogger logger;

  DataStreamProcessor processor(
    reader,
    assembler,
    signalProcessor,
    buffers,
    archive,
    logger);

  for (uint32_t m = 0; m < 3; ++m)
  {
    DataStreamAnchor anchor;

    anchor.module = ModuleId{m};
    anchor.firstFrameIndex = 0;
    anchor.startTimestamp = Timestamp{0};
    anchor.startWallTime = WallClockTime{0};
    anchor.frameRate = 1000.0;

    QVERIFY(processor.process(anchor));
  }

  std::vector<std::size_t> channelCounts = {2, 3, 2};

  for (uint32_t n = 0; n < 10; ++n)
  {
    for (uint32_t m = 0; m < 3; ++m)
    {
      DataBlock block;

      block.module = ModuleId{m};
      block.firstFrameIndex = n;
      block.frameRate = 1000.0;
      block.channelCount = channelCounts[m];
      block.frameCount = 1;

      block.values.resize(
        channelCounts[m]);

      for (std::size_t i = 0;
           i < channelCounts[m];
           ++i)
      {
        block.values[i] =
          static_cast<double>(
            n * 10 + m * 2);
      }

      QVERIFY(
        processor.process(block));
    }

    const auto k =
      static_cast<std::size_t>((n + 1) * 3);

    QCOMPARE(
      archive.frames.size(),
      std::size_t{k});

    QVERIFY(buffers.ready());

    const Frame& archived =
      archive.frames[k - 1];

    QCOMPARE(
      archived.number,
      FrameNumber{k - 1});

    QCOMPARE(
      archived.timestamp,
      Timestamp{n * 1'000});

    QCOMPARE(
      archived.wallTime,
      WallClockTime{n * 1'000});

    QCOMPARE(archived.raw().value(0), n * 10.0);
    QCOMPARE(archived.raw().value(1), n * 10.0);

    QCOMPARE(archived.raw().value(2), n * 10.0 + 2);
    QCOMPARE(archived.raw().value(3), n * 10.0 + 2);
    QCOMPARE(archived.raw().value(4), n * 10.0 + 2);

    QCOMPARE(archived.raw().value(5), n * 10.0 + 4);
    QCOMPARE(archived.raw().value(6), n * 10.0 + 4);


    Frame latest;
    QVERIFY(buffers.readFrame(latest));

    QCOMPARE(
      latest.number,
      archived.number);

    QCOMPARE(
      latest.timestamp,
      archived.timestamp);

    QCOMPARE(
      latest.wallTime,
      archived.wallTime);

    QVERIFY(
      latest.raw().equals(
        archived.raw().values()));
  }
}

void tst_dataarchive::
  test_DataStreamProcessor_block_with_multiple_frames()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  DataStreamReader reader;

  FrameAssembler assembler(
    cfg,
    layout,
    FrameStartPolicy::AllowIncomplete);

  BufferManager buffers;
  buffers.initialize(layout);

  FormulaAstRepository formulas;
  CalculationPlan plan;
  CalibrationRepository calibrations;

  SignalProcessor signalProcessor(
    layout,
    formulas,
    plan,
    calibrations);

  TestArchiveFrameWriter archive;
  TestLogger logger;

  DataStreamProcessor processor(
    reader,
    assembler,
    signalProcessor,
    buffers,
    archive,
    logger);

  DataStreamAnchor anchor;
  anchor.module = ModuleId{0};
  anchor.firstFrameIndex = 100;
  anchor.startTimestamp = Timestamp{1'000'000};
  anchor.startWallTime = WallClockTime{10'000'000};
  anchor.frameRate = 1000.0;

  QVERIFY(
    processor.process(anchor));

  DataBlock block;
  block.module = ModuleId{0};
  block.firstFrameIndex = 100;
  block.frameRate = 1000.0;
  block.channelCount = 2;
  block.frameCount = 4;

  block.values =
    {
      10, 11,   // frame 100
      20, 21,   // frame 101
      30, 31,   // frame 102
      40, 41    // frame 103
    };

  QVERIFY(
    processor.process(block));

  QCOMPARE(
    archive.frames.size(),
    std::size_t{4});

  for (std::size_t i = 0; i < 4; ++i)
  {
    const Frame& frame =
      archive.frames[i];

    QCOMPARE(
      frame.number,
      FrameNumber{i});

    QCOMPARE(
      frame.timestamp,
      Timestamp{
        1'000'000 +
        static_cast<uint64_t>(i) * 1000
      });

    QCOMPARE(
      frame.wallTime,
      WallClockTime{
        10'000'000 +
        static_cast<int64_t>(i) * 1000
      });

    QCOMPARE(
      frame.raw().value(0),
      static_cast<double>((i + 1) * 10));

    QCOMPARE(
      frame.raw().value(1),
      static_cast<double>((i + 1) * 10 + 1));

    // Остальные модули ещё ничего не прислали.
    for (uint32_t j = 2; j < 7; ++j)
      QVERIFY(std::isnan(
        frame.raw().value(j)));


    QVERIFY(buffers.ready());

    Frame latest;
    QVERIFY(buffers.readFrame(latest));

    QCOMPARE(
      latest.number,
      FrameNumber{3});

    QCOMPARE(
      latest.timestamp,
      Timestamp{1'003'000});

    QCOMPARE(
      latest.wallTime,
      WallClockTime{10'003'000});

    QCOMPARE(
      latest.raw().value(0),
      40.0);

    QCOMPARE(
      latest.raw().value(1),
      41.0);
  }
}

void tst_dataarchive::test_DataStreamWorker_base()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  DataStreamReader reader;

  FrameAssembler assembler(
    cfg,
    layout,
    FrameStartPolicy::AllowIncomplete);

  BufferManager buffers;
  buffers.initialize(layout);

  FormulaAstRepository formulas;
  CalculationPlan plan;
  CalibrationRepository calibrations;

  SignalProcessor signalProcessor(
    layout,
    formulas,
    plan,
    calibrations);

  TestArchiveFrameWriter archive;
  TestLogger logger;
  DataBlockQueue queue;

  DataStreamProcessor processor(
    reader,
    assembler,
    signalProcessor,
    buffers,
    archive,
    logger);

  DataStreamWorker worker(
    queue,
    processor,
    logger);

  queue.start();
  QVERIFY(worker.start());

  for (uint32_t m = 0; m < 3; ++m)
  {
    DataStreamAnchor anchor;

    anchor.module = ModuleId{m};
    anchor.firstFrameIndex = 0;
    anchor.startTimestamp = Timestamp{0};
    anchor.startWallTime = WallClockTime{0};
    anchor.frameRate = 1000.0;

    queue.startStream(anchor);
  }

  for (uint32_t n = 0; n < 10; ++n)
  {
    for (uint32_t m = 0; m < 3; ++m)
    {
      const auto channelCount =
        cfg.moduleChannelCount(
          ModuleId{m});

      std::vector<double> values(
        channelCount);

      for (std::size_t i = 0;
           i < channelCount;
           ++i)
      {
        values[i] =
          static_cast<double>(
            n * 10 + m * 2);
      }

      queue.push(
        ModuleId{m},
        n,
        values,
        channelCount,
        1,
        1000.0);
    }
  }

  queue.stop();
  worker.join();


  QVERIFY(
    !worker.isRunning());

  QCOMPARE(
    queue.size(),
    std::size_t{0});

  QCOMPARE(
    archive.frames.size(),
    std::size_t{30});

  const Frame& last =
    archive.frames.back();

  QCOMPARE(
    last.number,
    FrameNumber{29});

  QCOMPARE(
    last.timestamp,
    Timestamp{9'000});

  QCOMPARE(
    last.wallTime,
    WallClockTime{9'000});

  QCOMPARE(last.raw().value(0), 90.0);
  QCOMPARE(last.raw().value(1), 90.0);

  QCOMPARE(last.raw().value(2), 92.0);
  QCOMPARE(last.raw().value(3), 92.0);
  QCOMPARE(last.raw().value(4), 92.0);

  QCOMPARE(last.raw().value(5), 94.0);
  QCOMPARE(last.raw().value(6), 94.0);
}

void tst_dataarchive::test_LCardDataSource_to_archive()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  DataStreamReader reader;

  FrameAssembler assembler(
    cfg,
    layout,
    FrameStartPolicy::AllowIncomplete);

  BufferManager buffers;
  buffers.initialize(layout);

  FormulaAstRepository formulas;
  CalculationPlan plan;
  CalibrationRepository calibrations;

  SignalProcessor signalProcessor(
    layout,
    formulas,
    plan,
    calibrations);

  TestArchiveFrameWriter archive;
  TestLogger logger;

  DataBlockQueue queue;

  DataStreamProcessor processor(
    reader,
    assembler,
    signalProcessor,
    buffers,
    archive,
    logger);

  DataStreamWorker worker(
    queue,
    processor,
    logger);

  FakeClock clock;

  clock.setTimestamp(
    1'000'000);

  clock.setWallClockTime(
    10'000'000);

  constexpr std::size_t frameCount = 4;
  constexpr std::size_t channelCount = 2;
  constexpr uint32_t blockCount = 3;

  auto module =
    std::make_unique<SmartBlockLCardModule>(
      frameCount,
      channelCount,
      blockCount);

  auto* modulePtr =
    module.get();

  LCardDataSource source(
    ModuleId{0},
    2,
    std::move(module),
    clock,
    &queue,
    &queue);

  queue.start();

  QVERIFY(
    worker.start());

  QVERIFY(
    source.start());

  QTRY_COMPARE_WITH_TIMEOUT(
    modulePtr->remainingBlockCount(),
    uint32_t{0},
    1000);

  // Producer больше не должен работать.
  source.stop();

  // Закрываем очередь только после producer.
  queue.stop();

  // Consumer дочитывает всё накопленное.
  worker.join();

  QCOMPARE(
    queue.size(),
    std::size_t{0});

  QCOMPARE(
    archive.frames.size(),
    std::size_t{12});

  const Frame& first =
    archive.frames.front();

  QCOMPARE(
    first.number,
    FrameNumber{0});

  QCOMPARE(
    first.timestamp,
    Timestamp{1'000'000});

  QCOMPARE(
    first.wallTime,
    WallClockTime{10'000'000});

  QCOMPARE(
    first.raw().value(0),
    0.0);

  QCOMPARE(
    first.raw().value(1),
    1.0);

  // ==========================
  const Frame& last =
    archive.frames.back();

  QCOMPARE(
    last.number,
    FrameNumber{11});

  QCOMPARE(
    last.timestamp,
    Timestamp{1'011'000});

  QCOMPARE(
    last.wallTime,
    WallClockTime{10'011'000});

  QCOMPARE(
    last.raw().value(0),
    6.0);

  QCOMPARE(
    last.raw().value(1),
    7.0);
}

void tst_dataarchive::test_LCardDataSource_some_modules_to_archive()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Some_Modules();

  SignalMemoryLayout layout;
  layout.build(cfg);

  DataStreamReader reader;

  FrameAssembler assembler(
    cfg,
    layout,
    FrameStartPolicy::WaitForAllModules);

  BufferManager buffers;
  buffers.initialize(layout);

  FormulaAstRepository formulas;
  CalculationPlan plan;
  CalibrationRepository calibrations;

  SignalProcessor signalProcessor(
    layout,
    formulas,
    plan,
    calibrations);

  TestArchiveFrameWriter archive;
  TestLogger logger;

  DataBlockQueue queue;

  DataStreamProcessor processor(
    reader,
    assembler,
    signalProcessor,
    buffers,
    archive,
    logger);

  DataStreamWorker worker(
    queue,
    processor,
    logger);

  FakeClock clock;

  clock.setTimestamp(
    1'000);

  clock.setWallClockTime(
    10'000);

  constexpr std::size_t frameCount0 = 2;
  constexpr uint32_t blockCount0 = 3;

  auto module0 =
    std::make_unique<SmartBlockLCardModule>(
      frameCount0,
      cfg.moduleChannelCount(ModuleId{0}),
      blockCount0);

  auto* modulePtr0 =
    module0.get();

  LCardDataSource source0(
    ModuleId{0},
    cfg.moduleChannelCount(ModuleId{0}),
    std::move(module0),
    clock,
    &queue,
    &queue);

  constexpr std::size_t frameCount1 = 2;
  constexpr uint32_t blockCount1 = 2;

  auto module1 =
    std::make_unique<SmartBlockLCardModule>(
      frameCount1,
      cfg.moduleChannelCount(ModuleId{1}),
      blockCount1);

  auto* modulePtr1 =
    module1.get();

  LCardDataSource source1(
    ModuleId{1},
    cfg.moduleChannelCount(ModuleId{1}),
    std::move(module1),
    clock,
    &queue,
    &queue);

  constexpr std::size_t frameCount2 = 2;
  constexpr uint32_t blockCount2 = 3;

  auto module2 =
    std::make_unique<SmartBlockLCardModule>(
      frameCount2,
      cfg.moduleChannelCount(ModuleId{2}),
      blockCount2);

  auto* modulePtr2 =
    module2.get();

  LCardDataSource source2(
    ModuleId{2},
    cfg.moduleChannelCount(ModuleId{2}),
    std::move(module2),
    clock,
    &queue,
    &queue);


  queue.start();

  QVERIFY(
    worker.start());

  QVERIFY(
    source0.start());
  QVERIFY(
    source1.start());
  QVERIFY(
    source2.start());

  QTRY_COMPARE_WITH_TIMEOUT(
    modulePtr0->remainingBlockCount(),
    uint32_t{0},
    1000);
  QTRY_COMPARE_WITH_TIMEOUT(
    modulePtr1->remainingBlockCount(),
    uint32_t{0},
    1000);
  QTRY_COMPARE_WITH_TIMEOUT(
    modulePtr2->remainingBlockCount(),
    uint32_t{0},
    1000);

  // Producer-ы больше не должны работать.
  source0.stop();
  source1.stop();
  source2.stop();

  // Закрываем очередь только после producer-ов.
  queue.stop();

  // Consumer дочитывает всё накопленное.
  worker.join();


  QCOMPARE(
    queue.size(),
    std::size_t{0});

  QVERIFY(
    !archive.frames.empty());

  for (std::size_t i = 0;
       i < archive.frames.size();
       ++i)
  {
    const Frame& frame =
      archive.frames[i];

    QCOMPARE(
      frame.number,
      FrameNumber{
        static_cast<uint64_t>(i)
      });

    for (uint32_t n = 0;
         n < frame.raw().size();
         ++n)
    {
      QVERIFY(
        !std::isnan(
          frame.raw().value(n)));
    }
  }

  for (const Frame& frame :
       archive.frames)
  {
    QVERIFY(
      frame.timestamp.value >= 1'000);

    QVERIFY(
      frame.timestamp.value <= 6'000);

    QVERIFY(
      frame.wallTime.unixMicroseconds >=
      10'000);

    QVERIFY(
      frame.wallTime.unixMicroseconds <=
      15'000);
  }

  QVERIFY(
    archive.frames.size() <=
    std::size_t{16});

}

