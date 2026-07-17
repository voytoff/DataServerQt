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
  void test_ping_extraData();
  void test_invalid_packet();
  void test_packet_payloadSizeMismatch();
  void test_packet_payloadSizeTooSmall();
  void test_unknown_packet();
  void test_subscribeList_ok();
  void test_subscribeList_truncatedTagArray();
  void test_subscribeList_empty();
  void test_subscribeList_tooManyTags();
  void test_subscribeList_invalidTag();
  void test_subscribeList_invalidRate();
  void test_subscribeList_duplicateTag();
  void test_subscribeList_emptyPayload();
  void test_unsubscribe_ok();
  void test_unsubscribe_invalidId();
  void test_unsubscribe_extraData();
  void test_unsubscribe_emptyPayload();

};

#endif // TST_UDPSERVER_H
