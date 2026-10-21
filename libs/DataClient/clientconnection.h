#pragma once

#include "datatypes.h"
#include "protocol/subscriptionpackets.h"

#include <QObject>
#include <QHostAddress>
#include <QUdpSocket>

#include <span>
#include <vector>

namespace qds
{

class ClientConnection : public QObject
{
  Q_OBJECT

public:
  explicit ClientConnection(
    QObject* parent = nullptr);

  bool open(
    const QHostAddress& address,
    quint16 port,
    quint16 localPort = 0);

  bool subscribe(
    std::span<const SignalId> signalIds,
    PublishRate rate);

  void close();

signals:
  void subscribed(qds::SubscriptionId id);

  void dataReceived(
    qds::SubscriptionId id,
    uint64_t timestamp,
    std::vector<double> values);

  void errorOccurred(QString message);

private:
  void onReadyRead();

  QUdpSocket m_socket;

  QHostAddress m_serverAddress;
  quint16 m_serverPort = 0;

  std::vector<SignalId> m_pendingSignals;
};

}