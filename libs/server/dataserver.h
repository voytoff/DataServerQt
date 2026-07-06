#ifndef DATASERVER_H
#define DATASERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QTimer>

#include "dataengine.h"

namespace qds
{

class ClientSession;

class DataServer : public QTcpServer
{
  Q_OBJECT

public:

  explicit DataServer(DataEngine* engine, QObject* parent = nullptr);

  bool start(quint16 port);

protected:
  void incomingConnection(qintptr socketDescriptor) override;

private slots:
  void broadcastSnapshot();

private:
  DataEngine* m_engine = nullptr;
  QVector<ClientSession*> m_clients;
  QTimer m_timer;
};

}

#endif // DATASERVER_H
