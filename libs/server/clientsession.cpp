#include "clientsession.h"
#include "packetheader.h"
#include "packettype.h"

#include <QDataStream>
#include <algorithm>

namespace qds
{

ClientSession::ClientSession(qintptr socketDescriptor, QObject* parent)
    : QObject(parent)
{
    m_socket.setSocketDescriptor(socketDescriptor);

    connect(&m_socket, &QTcpSocket::readyRead,
            this, &ClientSession::onReadyRead);
}

QTcpSocket* ClientSession::socket()
{
    return &m_socket;
}

bool ClientSession::hasSubscription() const
{
    if (m_useRange)
        return m_to >= m_from;

    return !m_list.empty();
}

void ClientSession::onReadyRead()
{
    m_inBuffer.append(m_socket.readAll());
    processPacket();
}

void ClientSession::processPacket()
{
    while (m_inBuffer.size() >= (int)sizeof(PacketHeader))
    {
        auto* header = reinterpret_cast<const PacketHeader*>(m_inBuffer.data());

        if (header->magic != 0xDA7A)
        {
            m_inBuffer.clear();
            return;
        }

        if (m_inBuffer.size() < (int)(sizeof(PacketHeader) + header->size))
            return;

        QByteArray payload =
            m_inBuffer.mid(sizeof(PacketHeader), header->size);

        PacketType type = (PacketType)header->type;

        switch (type)
        {
        case PacketType::SubscribeRange:
            handleSubscribeRange(payload);
            break;

        case PacketType::SubscribeList:
            handleSubscribeList(payload);
            break;

        case PacketType::UnsubscribeAll:
            handleUnsubscribeAll();
            break;

        default:
            break;
        }

        m_inBuffer.remove(0, sizeof(PacketHeader) + header->size);
    }
}

void ClientSession::handleSubscribeRange(const QByteArray& payload) {
  qDebug() << "Subscribe range:" << m_from << m_to;
    QDataStream in(payload);

    in >> m_from >> m_to;

    m_useRange = true;
}

void ClientSession::handleSubscribeList(const QByteArray& payload)
{
    QDataStream in(payload);

    quint32 count;
    in >> count;

    m_list.clear();
    m_list.reserve(count);

    for (quint32 i = 0; i < count; ++i)
    {
        TagId id;
        in >> id;
        m_list.push_back(id);
    }

    m_useRange = false;
}

void ClientSession::handleUnsubscribeAll()
{
    m_list.clear();
    m_from = m_to = 0;
    m_useRange = false;
}

void ClientSession::sendSnapshotFiltered(const double* data, size_t size)
{
  qDebug() << "Sending snapshot";
  m_outBuffer.clear();

    QDataStream out(&m_outBuffer, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    quint32 count = 0;

    out << count; // placeholder

    if (m_useRange)
    {
        TagId end = std::min<TagId>(m_to, (TagId)size - 1);

        for (TagId i = m_from; i <= end; ++i)
        {
            out << i << data[i];
            ++count;
        }
    }
    else
    {
        for (TagId id : m_list)
        {
            if (id < size)
            {
                out << id << data[id];
                ++count;
            }
        }
    }

    // patch count (first 4 bytes)
    *reinterpret_cast<quint32*>(m_outBuffer.data()) = count;

    m_socket.write(m_outBuffer);
}

}