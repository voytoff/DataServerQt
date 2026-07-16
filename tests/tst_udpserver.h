#ifndef TST_UDPSERVER_H
#define TST_UDPSERVER_H

#include <QObject>

class tst_udpserver : public QObject
{
  Q_OBJECT
public:
  tst_udpserver();
  ~tst_udpserver() override;

private slots:
  void test_ping();
  void test_invalid_packet();
  void test_unknown_packet();
  void test_subscribeList_ok();
  //void test_subscribeList_empty();
  //void test_subscribeList_tooManyTags();
  //void test_subscribeList_invalidTag();
  //void test_subscribeList_duplicateTag();

};

#endif // TST_UDPSERVER_H
