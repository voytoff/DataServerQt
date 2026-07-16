#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>

#include "protocol/subscriptionpackets.h"
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
    const SystemConfiguration& configuration,
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
  const SystemConfiguration& m_configuration;
  SubscriptionManager& m_subscriptions;
  LiveScheduler& m_scheduler;
  void processSubscribeList(PacketReader &reader, const Endpoint &endpoint);
  void sendSubscribeResponse(const Endpoint& endpoint, SubscribeResult result, SubscriptionId id = {});
  void sendErrorResponse(const Endpoint& endpoint);
  SubscriptionId createSubscription(const Endpoint& endpoint, PublishRate rate, std::span<const TagId> tags);
};

}

#endif // UDPSERVER_H