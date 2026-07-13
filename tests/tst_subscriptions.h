#ifndef TST_SUBSCRIPTIONS_H
#define TST_SUBSCRIPTIONS_H

#include <QObject>
#include <QTest>

class tst_subscriptions : public QObject
{
  Q_OBJECT
public:
  tst_subscriptions();
  ~tst_subscriptions() override;

private slots:
  void test_subscriptions_add_remove();
  void test_subscriptions_unique_ids();
  void test_subscriptions_id_not_reused();

};

#endif // TST_SUBSCRIPTIONS_H
