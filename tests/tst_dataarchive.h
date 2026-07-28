#ifndef TST_DATAARCHIVE_H
#define TST_DATAARCHIVE_H

#include <QObject>

class tst_dataarchive : public QObject
{
  Q_OBJECT
public:
  tst_dataarchive();
  ~tst_dataarchive() override;

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

private:
  const std::string fileName = "test_file.dat";

};

#endif // TST_DATAARCHIVE_H
