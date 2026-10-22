#pragma once

#include <QObject>

class tst_dataclient : public QObject
{
  Q_OBJECT
public:
  tst_dataclient();
  ~tst_dataclient() override;

private slots:
  /** формирование и отправка подписки. */
  void subscribeRequest();
  /** получение подтверждения и сохранение идентификатора подписки. */
  void subscribeResponse();
  /** приём пакета LiveData и проверка массива double, времени измерения и SubscriptionId */
   void liveData();

  void unsubscribe();
  void subscribeRejected();

};

