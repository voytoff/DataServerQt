#ifndef TST_PUBLISHER_H
#define TST_PUBLISHER_H

#include <QObject>

class tst_publisher : public QObject
{
  Q_OBJECT
public:
  tst_publisher();
  ~tst_publisher() override;

private slots:
  // Подписка содержит TagId, которого нет в LiveStorage.
  void test_publish_invalidTag();
  // publish(... sequence = 10 ...) и проверить
  void test_publish_sequence();
  // проверка пустой подписки
  void test_publish_emptySubscription();
  void test_publish_reuseWriter();

};

#endif // TST_PUBLISHER_H
