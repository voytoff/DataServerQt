#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <qudpsocket.h>

#include "livescheduler.h"
#include "packetreader.h"
#include "subscriptionmanager.h"

namespace qds
{

class UdpServer : public QObject {
  Q_OBJECT

public:
  explicit UdpServer(
    SubscriptionManager& subscriptions,
    LiveScheduler& scheduler,
    QObject* parent = nullptr);

  bool start(quint16 port);
  void stop();

private slots:
  void onReadyRead();

private:
  void processPacket(
    PacketReader& reader,
    const Endpoint& endpoint);

private:
  QUdpSocket m_socket;

  SubscriptionManager& m_subscriptions;
  LiveScheduler& m_scheduler;
};

}

#endif // UDPSERVER_H
