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
  /** отмена подписки и повторная подписка. */
  void unsubscribe();
   /** обработка ошибки протокола и восстановление состояния. */
  void protocolError();
  /** отказ сервера в подписке и возможность повторного запроса. */
  void subscribeRejected();

  void subscribeTimeout();
  void unsubscribeTimeout();

  void subscribeRetry();
  void unsubscribeRetry();

  void dataTimeout();
  void autoRecovery();
  void cancelRecovery();
  void recoveryRejected();
  void recoveryTemporaryError_data();
  void recoveryTemporaryError();

  void invalidSubscriptionResponse();
  void invalidUnsubscribeResponse();
  void invalidLiveDataSubscription();
};

