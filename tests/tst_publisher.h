#pragma once

#include <QObject>

class tst_publisher : public QObject
{
  Q_OBJECT
public:
  tst_publisher();
  ~tst_publisher() override;

private slots:
  // publish(... sequence = 10 ...) и проверить
  //void test_publish_sequence();
  // проверка пустой подписки
  //void test_publish_emptySubscription();
  void test_publish_reuseWriter();
  void test_publish_failSignal();
  void test_publish_publishRate();

  void test_publish_phase_preserving();
  void test_publish_sendFailure();
  void test_publish_sendFailure_afterStart();

  void test_publish_zeroFrameRate();
  void test_publish_zeroSubscriptionRate();
  void test_publish_subscriptionRateAboveFrameRate();
  void test_publish_subscriptionRateNotDivisor();

  void test_publish_failSubscription();
};
