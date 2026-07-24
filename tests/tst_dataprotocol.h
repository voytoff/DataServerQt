#ifndef TST_DATAPROTOCOL_H
#define TST_DATAPROTOCOL_H

#include <QObject>

class test_dataprotocol : public QObject
{
  Q_OBJECT
public:
  test_dataprotocol();
  ~test_dataprotocol() override;

private slots:
  void test_subscribeListPacket();
  void test_subscribeResponsePacket();

};

#endif // TST_DATAPROTOCOL_H
