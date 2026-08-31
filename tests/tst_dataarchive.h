#pragma once

#include <QObject>

class tst_dataarchive : public QObject
{
  Q_OBJECT
public:
  tst_dataarchive();
  ~tst_dataarchive() override;

private slots:
  void test_archivewriter_create_open();
  void test_archivewriter_writeOneRecord();
  void test_archivewriter_writeSomeRecords();
  void test_archivewriter_badChannelCount();
  void test_archivewriter_writeAfterClose();
  void test_archivewriter_doubleClose();

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

  void test_archiveDescription_writeRawSignals();
  void test_archiveDescription_writeCalculatedSignals();
  void test_archiveDescription_writeMultipleFiles();

  void test_archiveDescription_archiveDescriptionBuilder();  

};
