#pragma once

#include "datatypes.h"
#include "dataclient_global.h"

#include <QObject>
#include <QHostAddress>
#include <QUdpSocket>
#include <QTimer>

#include <cstdint>
#include <span>
#include <vector>

namespace qds
{

class DATACLIENT_EXPORT ClientConnection : public QObject
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

  bool unsubscribe();

  void close();

  bool isOpen() const noexcept;

  SubscriptionId subscriptionId() const noexcept;

  const std::vector<SignalId>& signalIds() const noexcept;

private:
  void onReadyRead();
  void onRequestTimeout();
  RequestId nextRequestId() noexcept;

  bool sendPendingPacket();

private:
  static constexpr int RequestTimeoutMs = 1000;

  QUdpSocket m_socket;
  QTimer m_requestTimer;

  QHostAddress m_serverAddress;
  quint16 m_serverPort = 0;

  SubscriptionId m_subscriptionId{};

  std::vector<SignalId> m_pendingSignals;
  std::vector<SignalId> m_signals;

  bool m_subscriptionPending = false;
  bool m_unsubscribePending = false;

  RequestId m_nextRequestId{1};
  RequestId m_pendingRequestId{};

  static constexpr int MaxRequestRetries = 3;

  std::vector<std::byte> m_pendingPacket;
  int m_retryCount = 0;

signals:
  void subscribed(qds::SubscriptionId id);

  void unsubscribed();

  void dataReceived(
    qds::SubscriptionId id,
    uint64_t timestamp,
    std::vector<double> values);

  void errorOccurred(QString message);

};

}