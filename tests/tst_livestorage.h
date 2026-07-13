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
  void test_livePublisher();
  void test_livePublisher_reverse();

};

#endif // TST_LIVESTORAGE_H
