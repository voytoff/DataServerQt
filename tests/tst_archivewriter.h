#ifndef TST_ARCHIVEWRITER_H
#define TST_ARCHIVEWRITER_H

#include <QObject>

class tst_archivewriter : public QObject
{
  Q_OBJECT
public:
  tst_archivewriter();
  ~tst_archivewriter() override;

private slots:
  /*
    create/open
    write один record
    write несколько records
    проверить lastTimestamp
    проверить recordCount (его мы еще добавим в header)
    проверить чтение через будущий ArchiveReader
   */
  void test_archivewriter_create_open();
  void test_archivewriter_writeOneRecord();

};

#endif // TST_ARCHIVEWRITER_H
