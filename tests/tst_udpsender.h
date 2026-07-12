#ifndef TST_UDPSENDER_H
#define TST_UDPSENDER_H

#include <QObject>
#include <QTest>

class tst_udpsender: public QObject
{
  Q_OBJECT
public:
  tst_udpsender();
  ~tst_udpsender() override;

private slots:
  void test_send();
  void test_send_bad_address();
  void test_send_empty();
};

#endif // TST_UDPSENDER_H
