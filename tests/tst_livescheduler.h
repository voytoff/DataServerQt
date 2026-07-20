#ifndef TST_LIVESCHEDULER_H
#define TST_LIVESCHEDULER_H

#include <QObject>

class tst_livescheduler : public QObject
{
  Q_OBJECT
public:
  tst_livescheduler();
  ~tst_livescheduler() override;

private slots:
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


  void test_publishFailed_sequenceNotIncremented();
  void test_emptySubscription_sequenceIncremented();
  void test_publishPacket_singleTag();

};

#endif // TST_LIVESCHEDULER_H
