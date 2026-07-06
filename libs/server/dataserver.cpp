#include "dataserver.h"
#include "clientsession.h"

#include <QDataStream>
#include <QTimer>

namespace qds
{

DataServer::DataServer(DataEngine* engine, QObject* parent)
    : QTcpServer(parent)
    , m_engine(engine)
{
    connect(&m_timer, &QTimer::timeout,
            this, &DataServer::broadcastSnapshot);

    m_timer.start(1); // до 1000 Hz (1 ms tick)
}

bool DataServer::start(quint16 port)
{
    return listen(QHostAddress::Any, port);
}

void DataServer::incomingConnection(qintptr socketDescriptor)
{
    auto* client = new ClientSession(socketDescriptor, this);
    m_clients.push_back(client);
}

void DataServer::broadcastSnapshot()
{
    // 1. вычисления
    m_engine->process();

    const double* data = m_engine->memory().data();
    size_t size = m_engine->memory().size();

    // 2. отправка клиентам
    for (auto* client : std::as_const(m_clients))
    {
        if (!client->hasSubscription())
            continue;

        client->sendSnapshotFiltered(data, size);
    }
}

}