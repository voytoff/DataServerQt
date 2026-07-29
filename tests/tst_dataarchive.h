#ifndef TST_DATAARCHIVE_H
#define TST_DATAARCHIVE_H

#include "archivefile.h"
#include "archiveformat.h"
#include <QObject>
#include <filesystem>

const std::string fileName = "test_file.dat";
const uint64_t firstTimestamp = 1234567u;
const uint64_t timestamp = 7654321u;

class tst_dataarchive : public QObject
{
  Q_OBJECT
public:
  tst_dataarchive();
  ~tst_dataarchive() override;

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
      .firstTimestamp = firstTimestamp
    };
    return hdr;
  }
  static qds::ArchiveFile getAfterCreateArchiveFile() {
    using namespace qds;
    DataFileHeader hdr = getDataFileHeader();

    std::string filePath = getFilePath(fileName);

    ArchiveFile file;
    file.create(filePath, hdr);

    return file;
  }


private slots:
  void test_dataFileHeader_headerSize();
  void test_dataFileHeader_base();
  void test_archiveFile_emptyFile();
  void test_archiveFile_badMagic();
  void test_archiveFile_invalidVersion();
  void test_archiveFile_invalidRecordSize();
  void test_archiveFile_invalidHeaderSize();
  void test_archiveFile_badChannelCount();
  void test_archiveFile_reCreates();
  void test_archiveFile_saveHeader();
  void test_archiveFile_saveHeaderInReadMode();
  void test_archiveFile_seekToZerro();
  void test_archiveFile_seekToDataAfterHeader();
  void test_archiveFile_saveZerroArray();
  void test_archiveFile_checkFileSize();
  void test_archiveFile_saveHeaderNotChangePosition();

};

#endif // TST_DATAARCHIVE_H
