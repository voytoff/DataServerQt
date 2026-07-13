#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>

#include "subscriptionmanager.h"
#include "livescheduler.h"
#include "packetreader.h"

namespace qds
{

class UdpServer : public QObject
{
  Q_OBJECT

public:
  // Конструктор
  explicit UdpServer(
    SubscriptionManager& subscriptions,
    LiveScheduler& scheduler,
    QObject* parent = nullptr);

  // Запуск
  bool start(uint16_t port);
  // Остановка
  void stop();

  uint16_t port() const noexcept;
  bool isRunning() const noexcept;

private slots:
  // Приём датаграмм
  void onReadyRead();

private:
  // Switch
  void processPacket(
    PacketReader& reader,
    const Endpoint& endpoint);

  void processPing(
    PacketReader& reader,
    const Endpoint& endpoint);

private:
  QUdpSocket m_socket;

  SubscriptionManager& m_subscriptions;
  LiveScheduler& m_scheduler;
};

}

#endif // UDPSERVER_H