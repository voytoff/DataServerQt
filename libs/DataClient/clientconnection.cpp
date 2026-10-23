#include "clientconnection.h"

#include "packetreader.h"
#include "packetwriter.h"

#include "protocol/publishheader.h"
#include "protocol/errorpackets.h"
#include "subscriptionpackets.h"
#include <qdebug.h>

namespace qds
{

ClientConnection::ClientConnection(
  QObject* parent)
  : QObject(parent)
{
  connect(
    &m_socket,
    &QUdpSocket::readyRead,
    this,
    &ClientConnection::onReadyRead);

  m_requestTimer.setSingleShot(true);

  connect(
    &m_requestTimer,
    &QTimer::timeout,
    this,
    &ClientConnection::onRequestTimeout);
}

bool ClientConnection::open(
  const QHostAddress& address,
  quint16 port,
  quint16 localPort)
{
  close();

  if (address.isNull() || port == 0)
    return false;

  if (!m_socket.bind(
        QHostAddress::AnyIPv4,
        localPort))
  {
    emit errorOccurred(
      m_socket.errorString());

    return false;
  }

  m_serverAddress = address;
  m_serverPort = port;

  return true;
}

bool ClientConnection::isOpen() const noexcept
{
  return m_socket.state() ==
           QAbstractSocket::BoundState &&
         m_serverPort != 0;
}

SubscriptionId ClientConnection::subscriptionId() const noexcept
{
  return m_subscriptionId;
}

const std::vector<SignalId>&
ClientConnection::signalIds() const noexcept
{
  return m_signals;
}

bool ClientConnection::subscribe(
  std::span<const SignalId> signalIds,
  PublishRate rate)
{
  if (!isOpen() ||
      m_subscriptionPending ||
      m_unsubscribePending ||
      m_subscriptionId.value != 0 ||
      signalIds.empty() ||
      signalIds.size() > MaxSubscriptionSignals)
  {
    return false;
  }

  switch (rate)
  {
  case PublishRate::Hz1:
  case PublishRate::Hz10:
  case PublishRate::Hz100:
    break;

  default:
    return false;
  }

  PacketWriter writer;
  const RequestId requestId = nextRequestId();

  writer.begin(
    PacketType::SubscribeListRequest);

  writer.write(
    SubscribeListRequest{
      .requestId = requestId,
      .rate = rate,
      .signalCount =
      static_cast<uint32_t>(
        signalIds.size())
    });

  writer.writeArray(
    signalIds.data(),
    signalIds.size());

  m_pendingPacket.assign(
    writer.span().begin(),
    writer.span().end());

  m_retryCount = 0;

  if (!sendPendingPacket())
  {
    m_pendingPacket.clear();

    emit errorOccurred(m_socket.errorString());
    return false;
  }

  m_pendingSignals.assign(
    signalIds.begin(),
    signalIds.end());

  m_pendingRequestId = requestId;
  m_subscriptionPending = true;

  m_requestTimer.start(RequestTimeoutMs);

  return true;
}

bool ClientConnection::unsubscribe()
{
  if (!isOpen() ||
      m_subscriptionId.value == 0 ||
      m_subscriptionPending ||
      m_unsubscribePending)
  {
    return false;
  }

  PacketWriter writer;
  const RequestId requestId = nextRequestId();

  writer.begin(
    PacketType::UnsubscribeRequest);

  writer.write(
    UnsubscribeRequest{
      .requestId = requestId,
      .id = m_subscriptionId
    });

  m_pendingPacket.assign(
    writer.span().begin(),
    writer.span().end());

  m_retryCount = 0;

  if (!sendPendingPacket())
  {
    m_pendingPacket.clear();

    emit errorOccurred(m_socket.errorString());
    return false;
  }

  m_pendingRequestId = requestId;
  m_unsubscribePending = true;

  m_requestTimer.start(RequestTimeoutMs);

  return true;
}

void ClientConnection::close()
{
  m_requestTimer.stop();

  m_socket.close();

  m_serverAddress = {};
  m_serverPort = 0;

  m_subscriptionId = {};

  m_pendingSignals.clear();
  m_signals.clear();

  m_pendingRequestId = {};
  m_subscriptionPending = false;
  m_unsubscribePending = false;

  m_pendingPacket.clear();
  m_retryCount = 0;
}

void ClientConnection::onReadyRead()
{
  while (m_socket.hasPendingDatagrams())
  {
    QByteArray datagram;

    datagram.resize(
      static_cast<qsizetype>(
        m_socket.pendingDatagramSize()));

    QHostAddress senderAddress;
    quint16 senderPort = 0;

    const qint64 received =
      m_socket.readDatagram(
        datagram.data(),
        datagram.size(),
        &senderAddress,
        &senderPort);

    if (received < 0)
      continue;

    if (senderAddress != m_serverAddress ||
        senderPort != m_serverPort)
    {
      continue;
    }

    PacketReader reader;

    reader.append(
      reinterpret_cast<const std::byte*>(
        datagram.constData()),
      static_cast<std::size_t>(
        received));

    if (!reader.nextPacket())
      continue;

    switch (reader.packetType())
    {
    case PacketType::SubscribeResponse:
    {
      SubscribeResponse response{};

      if (!reader.read(response) ||
          reader.remaining() != 0 ||
          reader.trailingBytes() != 0)
      {
        break;
      }

      if (!m_subscriptionPending ||
          response.requestId != m_pendingRequestId)
      {
        break;
      }

      m_requestTimer.stop();
      m_pendingRequestId = {};
      m_subscriptionPending = false;

      m_pendingPacket.clear();
      m_retryCount = 0;

      if (response.result != SubscribeResult::Ok)
      {
        m_pendingSignals.clear();

        emit errorOccurred(
          tr("Сервер отклонил подписку, код: %1")
            .arg(static_cast<int>(
              response.result)));

        break;
      }

      m_subscriptionId = response.id;

      m_signals =
        std::move(m_pendingSignals);

      m_pendingSignals.clear();

      emit subscribed(response.id);

      break;
    }

    case PacketType::UnsubscribeResponse:
    {
      UnsubscribeResponse response{};

      if (!reader.read(response) ||
          reader.remaining() != 0 ||
          reader.trailingBytes() != 0)
      {
        break;
      }

      if (!m_unsubscribePending ||
          response.requestId != m_pendingRequestId)
      {
        break;
      }

      m_requestTimer.stop();
      m_pendingRequestId = {};
      m_unsubscribePending = false;

      m_pendingPacket.clear();
      m_retryCount = 0;

      if (response.result !=
          UnsubscribeResult::Ok)
      {
        emit errorOccurred(
          tr("Сервер отклонил отписку"));

        break;
      }

      m_subscriptionId = {};
      m_signals.clear();

      emit unsubscribed();

      break;
    }

    case PacketType::LiveData:
    {
      PublishHeader header{};

      if (!reader.read(header))
        break;

      if (m_subscriptionId.value == 0 ||
          header.subscriptionId != m_subscriptionId ||
          header.valueCount != m_signals.size() ||
          header.valueCount > MaxSubscriptionSignals)
      {
        break;
      }

      std::vector<double> values(
        header.valueCount);

      if (!reader.readArray(
            values.data(),
            values.size()))
      {
        break;
      }

      if (reader.remaining() != 0 ||
          reader.trailingBytes() != 0)
      {
        break;
      }

      emit dataReceived(
        header.subscriptionId,
        header.timestamp,
        std::move(values));

      break;
    }

    case PacketType::ErrorResponse:
    {
      ErrorResponse response{};

      if (!reader.read(response) ||
          reader.remaining() != 0 ||
          reader.trailingBytes() != 0)
      {
        break;
      }

      m_pendingRequestId = {};

      if (m_subscriptionPending)
      {
        m_requestTimer.stop();

        m_subscriptionPending = false;
        m_pendingSignals.clear();
      }
      else if (m_unsubscribePending)
      {
        m_requestTimer.stop();

        m_unsubscribePending = false;
      }

      m_pendingPacket.clear();
      m_retryCount = 0;

      emit errorOccurred(
        tr("Ошибка протокола: %1, info=%2")
          .arg(static_cast<int>(response.code))
          .arg(response.info));

      break;
    }

    default:
      break;
    }
  }
}

void ClientConnection::onRequestTimeout()
{
  if (!m_subscriptionPending &&
      !m_unsubscribePending)
  {
    return;
  }

  if (m_retryCount < MaxRequestRetries)
  {
    ++m_retryCount;

    if (sendPendingPacket())
    {
      m_requestTimer.start(RequestTimeoutMs);
      return;
    }

    // Ошибка отправки — прекращаем повторы
    const QString error = m_socket.errorString();

    m_requestTimer.stop();
    m_pendingRequestId = {};
    m_pendingPacket.clear();
    m_retryCount = 0;

    m_subscriptionPending = false;
    m_unsubscribePending = false;
    m_pendingSignals.clear();

    emit errorOccurred(error);
    return;
  }

  m_pendingRequestId = {};
  m_pendingPacket.clear();
  m_retryCount = 0;

  if (m_subscriptionPending)
  {
    m_subscriptionPending = false;
    m_pendingSignals.clear();

    emit errorOccurred(
      tr("Превышено время ожидания подтверждения подписки"));

    return;
  }

  if (m_unsubscribePending)
  {
    m_unsubscribePending = false;

    emit errorOccurred(
      tr("Превышено время ожидания подтверждения отписки"));
  }
}

RequestId ClientConnection::nextRequestId() noexcept
{
  if (m_nextRequestId.value == 0)
    m_nextRequestId.value = 1;

  return {m_nextRequestId.value++};
}

bool ClientConnection::sendPendingPacket()
{
  if (!isOpen() || m_pendingPacket.empty())
    return false;

  const qint64 written =
    m_socket.writeDatagram(
      reinterpret_cast<const char*>(
        m_pendingPacket.data()),
      static_cast<qint64>(
        m_pendingPacket.size()),
      m_serverAddress,
      m_serverPort);

  return written ==
         static_cast<qint64>(
           m_pendingPacket.size());
}

}