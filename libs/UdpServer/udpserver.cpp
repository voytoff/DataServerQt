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

    Endpoint endpoint {
      .address = address.toString().toStdString(),
      .port = port
    };

    {
      /// !!!
      std::span<const std::byte> data1(
        reinterpret_cast<const std::byte*>(data.constData()),
        data.size());

      m_dispatcher.dispatch(data1, endpoint);
    }

    PacketReader reader;

    reader.append(
      reinterpret_cast<const std::byte*>(data.constData()),
      data.size());

    if (!reader.nextPacket())
    {
      sendErrorResponse(endpoint, ErrorCode::InvalidPacket);
      continue;
    }

    processPacket(reader, endpoint);
  }
}

void UdpServer::processPacket(PacketReader& reader, const Endpoint& endpoint)
{
  switch (reader.packetType())
  {
  case PacketType::SubscribeListRequest:
    processSubscribeList(reader, endpoint);
    break;

  case PacketType::UnsubscribeRequest:
    processUnsubscribe(reader, endpoint);
    break;

  case PacketType::Ping:
    processPing(reader, endpoint);
    break;

  default:
    sendErrorResponse(endpoint, ErrorCode::UnsupportedPacket);
    return;
  }
}

void UdpServer::processPing(PacketReader& reader, const Endpoint& endpoint)
{
  if (!checkEof(reader, endpoint))
    return;

  PacketWriter writer;
  writer.begin(PacketType::Pong);

  const QHostAddress address(
    QString::fromStdString(
      endpoint.address));

  //const auto sent =
  m_socket.writeDatagram(
    reinterpret_cast<const char*>(
      writer.data()),
    static_cast<qint64>(
      writer.size()),
    address,
    endpoint.port);

  //qDebug() << sent;
}

void UdpServer::processSubscribeList(PacketReader &reader, const Endpoint &endpoint)
{
  // 1. Разбор пакета
  SubscribeListRequest req;
  if (!readRequest(reader, endpoint, req))
    return;

  // 2. Проверка формата
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


  // 3. Проверка бизнес-логики
  std::vector<TagId> tags(req.tagCount);
  if (!reader.readArray(tags.data(), tags.size()))
  {
    sendErrorResponse(endpoint, ErrorCode::InvalidRequest);
    return;
  }

  if (!checkEof(reader, endpoint))
    return;

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


  // 4. Создание подписки
  SubscriptionId id =
    createSubscription(
      endpoint,
      req.rate,
      tags);

  // 5. Ответ клиенту
  sendSubscribeResponse(
    endpoint,
    SubscribeResult::Ok,
    id);

}

void UdpServer::processUnsubscribe(PacketReader &reader, const Endpoint &endpoint)
{
  UnsubscribeRequest req;
  if (!readRequest(reader, endpoint, req))
    return;

  if (!checkEof(reader, endpoint))
    return;

  const Subscription* sub = m_subscriptions.find(req.id);

  if (!sub)
  {
    sendUnsubscribeResponse(endpoint, UnsubscribeResult::InvalidId);
    return;
  }

  m_scheduler.removeSubscription(req.id);

  if (!m_subscriptions.remove(req.id)) {
    // не найдена подписка, уже удалена
    //sendUnsubscribeResponse(endpoint, UnsubscribeResult::InvalidId);
    //return;
  }

  sendUnsubscribeResponse(endpoint, UnsubscribeResult::Ok);
}

void UdpServer::sendUnsubscribeResponse(const Endpoint &endpoint, UnsubscribeResult result)
{
  PacketWriter writer;

  writer.begin(PacketType::UnsubscribeResponse);

  UnsubscribeResponse response;
  response.result = result;

  writer.write(response);

  const QHostAddress address(QString::fromStdString(endpoint.address));

  const auto sent = m_socket.writeDatagram(
    reinterpret_cast<const char*>(writer.data()),
    static_cast<qint64>(writer.size()),
    address,
    endpoint.port);

  if (sent != qint64(writer.size()))
  {
    qWarning() << "Не удалось отправить UDP-дейтаграмму полностью:" << sent << "из" << writer.size() << "байт";
  }

  //qDebug() << sent;
}

bool UdpServer::sendPacket(const Endpoint& endpoint, const PacketWriter& writer)
{
  const auto sent = m_socket.writeDatagram(
    reinterpret_cast<const char*>(writer.data()),
    static_cast<qint64>(writer.size()),
    QHostAddress(QString::fromStdString(endpoint.address)),
    endpoint.port);

  if (sent < 0)
  {
    qWarning() << "Не удалось отправить UDP-дейтаграмму:" << m_socket.errorString();
    return false;
  }

  return true;
}

bool UdpServer::checkEof(PacketReader &reader, const Endpoint &endpoint)
{
  if (reader.remaining() == 0 && reader.trailingBytes() == 0)
    return true;

  sendErrorResponse(
    endpoint,
    ErrorCode::ExtraData,
    reader.remaining());

  return false;
}

void UdpServer::sendSubscribeResponse(const Endpoint &endpoint, SubscribeResult result, SubscriptionId id)
{
  PacketWriter writer;

  writer.begin(PacketType::SubscribeResponse);

  SubscribeResponse response;
  response.result = result;
  response.id = id;

  writer.write(response);

  sendPacket(endpoint, writer);
}

void UdpServer::sendErrorResponse(const Endpoint& endpoint, ErrorCode code, uint32_t info)
{
  PacketWriter writer;
  writer.begin(PacketType::ErrorResponse);
  ErrorResponse response{.code = code, .info = info};
  writer.write(response);

  sendPacket(endpoint, writer);
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