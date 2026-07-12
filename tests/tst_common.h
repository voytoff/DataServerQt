#ifndef TST_COMMON_H
#define TST_COMMON_H

#include <QObject>
#include <QTest>

class test_common : public QObject
{
  Q_OBJECT

public:
  test_common();
  ~test_common() override;

private slots:
  void test_configuration_moduleTags();
  void test_livestorage();
  void test_subscriptions_add_remove();
  void test_subscriptions_unique_ids();
  void test_subscriptions_id_not_reused();
  void test_liveDataHeader();
  void test_LiveDataPacket();
  void test_liveDataPayload();
  void test_livePublisher();
  void test_livePublisher_reverse();
  void test_scheduler_100Hz();
  void test_scheduler_10Hz();
  void test_scheduler_1Hz();
  void test_scheduler_100Hz_remove_sub();
  void test_scheduler_AllHz();
  void test_scheduler_100Hz_period();
  void test_scheduler_10Hz_period();
  void test_scheduler_1Hz_period();
  void test_scheduler_send_subscription_sequence();
  void test_scheduler_send_subscriptions_sequence2();

};

#endif // TST_COMMON_H
