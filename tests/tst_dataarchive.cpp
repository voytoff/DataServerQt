#include "tst_dataarchive.h"
#include "archivedescription.h"
#include "archivedescriptionbuilder.h"
#include "archivedescriptionwriter.h"
#include "archivefile.h"
#include "archiveformat.h"
#include "archivewriter.h"
#include "moduletype.h"
#include "testsrv.h"
#include <QtTest/qtestcase.h>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <array>

tst_dataarchive::tst_dataarchive() { }
tst_dataarchive::~tst_dataarchive() = default;


void tst_dataarchive::test_archivewriter_create_open()
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

void tst_dataarchive::test_archivewriter_writeOneRecord()
{
  using namespace qds;
  ArchiveWriter writer;
  auto filePath = tst_dataarchive::getFilePath(fileName);
  DataFileHeader hdr = tst_dataarchive::getDataFileHeader();

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
  auto filePath = tst_dataarchive::getFilePath(fileName);
  DataFileHeader hdr = tst_dataarchive::getDataFileHeader();

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
  auto filePath = tst_dataarchive::getFilePath(fileName);
  DataFileHeader hdr = tst_dataarchive::getDataFileHeader();

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
  auto filePath = tst_dataarchive::getFilePath(fileName);
  DataFileHeader hdr = tst_dataarchive::getDataFileHeader();

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
  auto filePath = tst_dataarchive::getFilePath(fileName);
  DataFileHeader hdr = tst_dataarchive::getDataFileHeader();

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
    tst_dataarchive::getFilePath(
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
    tst_dataarchive::getFilePath(
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
    tst_dataarchive::getFilePath(
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
    tst_dataarchive::getFilePath(
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