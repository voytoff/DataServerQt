#include "udpserver.h"
#include "protocol/subscriptionpackets.h"
#include "systemconfiguration.h"
#include <qdebug.h>

namespace qds
{

UdpServer::UdpServer(const SystemConfiguration &configuration,
                     SubscriptionManager& subscriptions,
                     LiveScheduler& scheduler,
                     QObject* parent)
  : QObject(parent)
  , m_subscriptions(subscriptions)
  , m_scheduler(scheduler)
  , m_configuration(configuration)
{
}

bool UdpServer::start(uint16_t port)
{
  connect(&m_socket, &QUdpSocket::readyRead, this, &UdpServer::onReadyRead);

  bool ok = m_socket.bind(QHostAddress::AnyIPv4, port);

  qDebug() << "bind:" << ok << m_socket.localAddress() << m_socket.localPort();
  if (!ok)  qDebug() << m_socket.errorString();

  return ok;
}

void UdpServer::stop()
{
  m_socket.close();
}

uint16_t UdpServer::port() const noexcept {
  return m_socket.localPort();
}

bool UdpServer::isRunning() const noexcept {
  return m_socket.state() == QAbstractSocket::BoundState;
}

void UdpServer::onReadyRead()
{
  qDebug() << "onReadyRead";
  while (m_socket.hasPendingDatagrams())
  {
    QByteArray data;
    data.resize(
      m_socket.pendingDatagramSize());

    QHostAddress address;
    quint16 port = 0;

    m_socket.readDatagram(
      data.data(),
      data.size(),
      &address,
      &port);

    Endpoint endpoint;
    endpoint.address =
      address.toString().toStdString();
    endpoint.port = port;

    PacketReader reader;

    reader.append(
      reinterpret_cast<const std::byte*>(
        data.constData()),
      data.size());

    if (!reader.nextPacket())
      continue;

    processPacket(reader, endpoint);
  }
}

void UdpServer::processPacket(
  PacketReader& reader,
  const Endpoint& endpoint)
{
  switch (reader.packetType())
  {
  case PacketType::SubscribeListRequest:
    processSubscribeList(reader, endpoint);
    break;

  case PacketType::Ping:
    processPing(reader, endpoint);
    break;

  default:
    qDebug() << "Unknown packet type:" << static_cast<int>(reader.packetType());
    break;
  }
}

void UdpServer::processPing(
  PacketReader&,
  const Endpoint& endpoint)
{
  qDebug() << "processPing";

  PacketWriter writer;
  writer.begin(PacketType::Pong);

  const QHostAddress address(
    QString::fromStdString(
      endpoint.address));

  const auto sent = m_socket.writeDatagram(
    reinterpret_cast<const char*>(
      writer.data()),
    static_cast<qint64>(
      writer.size()),
    address,
    endpoint.port);

  qDebug() << sent;
}

void UdpServer::processSubscribeList(PacketReader &reader, const Endpoint &endpoint)
{
  qDebug() << "processSubscribeList";

  SubscribeListRequest req;
  if (!reader.read(req))
  {
    sendErrorResponse(endpoint);
    return;
  }

  if (req.tagCount == 0) {
    sendSubscribeResponse(
      endpoint,
      SubscribeResult::EmptyList);

    return;
  }

  if (req.tagCount > MaxSubscriptionTags) {
    sendSubscribeResponse(
      endpoint,
      SubscribeResult::TooManyTags);

    return;
  }

  switch (req.rate)
  {
  case PublishRate::Hz1:
  case PublishRate::Hz10:
  case PublishRate::Hz100:
    break;

  default:
    sendSubscribeResponse(
      endpoint,
      SubscribeResult::InvalidRate);
    return;
  }

  std::vector<TagId> tags(req.tagCount);
  if (!reader.readArray(tags.data(), tags.size()))
  {
    sendErrorResponse(endpoint);
    return;
  }

  if (!reader.eof()) {
    sendErrorResponse(endpoint);
    return;
  }

  // неверный тег
  for (const TagId& tag : tags)
  {
    if (!m_configuration.containsTag(tag))
    {
      sendSubscribeResponse(
        endpoint,
        SubscribeResult::InvalidTag);

      return;
    }
  }

  // повторяющийся тег
  for (size_t i = 0; i < tags.size(); ++i)
  {
    for (size_t j = i + 1; j < tags.size(); ++j)
    {
      if (tags[i] == tags[j])
      {
        sendSubscribeResponse(
          endpoint,
          SubscribeResult::DuplicateTag);

        return;
      }
    }
  }

  SubscriptionId id =
    createSubscription(
      endpoint,
      req.rate,
      tags);

  sendSubscribeResponse(
    endpoint,
    SubscribeResult::Ok,
    id);

}

void UdpServer::sendSubscribeResponse(const Endpoint &endpoint, SubscribeResult result, SubscriptionId id)
{
  PacketWriter writer;

  writer.begin(PacketType::SubscribeResponse);

  SubscribeResponse response;
  response.result = result;
  response.id = id;

  writer.write(response);

  const QHostAddress address(
    QString::fromStdString(
      endpoint.address));

  const auto sent = m_socket.writeDatagram(
    reinterpret_cast<const char*>(writer.data()),
    static_cast<qint64>(writer.size()),
    address,
    endpoint.port);

  Q_ASSERT(sent == qint64(writer.size()));

  qDebug() << sent;
}

void UdpServer::sendErrorResponse(const Endpoint &endpoint)
{
  PacketWriter writer;
  writer.begin(PacketType::ErrorResponse);
}

SubscriptionId UdpServer::createSubscription(const Endpoint &endpoint, PublishRate rate, std::span<const TagId> tags)
{
  Subscription s;
  s.endpoint = endpoint;
  s.rate = rate;
  s.tags.assign(tags.begin(), tags.end());

  SubscriptionId id = m_subscriptions.add(s);

  m_scheduler.addSubscription(id, rate);

  return id;
}

}