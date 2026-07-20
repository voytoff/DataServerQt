#ifndef TST_LIVESTORAGE_H
#define TST_LIVESTORAGE_H

#include <QObject>

class tst_livestorage : public QObject
{
  Q_OBJECT
public:
  tst_livestorage();
  ~tst_livestorage() override;

private slots:
  void test_livestorage();
  void test_liveDataHeader();
  void test_LiveDataPacket();
  void test_liveDataPayload();
  void test_publishSamples();
  void test_publishSamples_reverseOrder();

  void test_updateModule_read();
  void test_updateModule_invalidSize_keepsPreviousValues();
  void test_read_invalidTag();
  void test_updateModule_multipleUpdates();

};

#endif // TST_LIVESTORAGE_H
