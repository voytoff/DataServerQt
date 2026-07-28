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

private:
  const std::string fileName = "test_file.dat";

};

#endif // TST_DATAARCHIVE_H
