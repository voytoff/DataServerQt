#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include "datatypes.h"
#include <QTcpSocket>
#include <QObject>
#include <vector>

namespace qds
{

class ClientSession : public QObject
{
  Q_OBJECT

public:

  explicit ClientSession(qintptr socketDescriptor, QObject* parent = nullptr);

  QTcpSocket* socket();

  bool hasSubscription() const;

  void sendSnapshotFiltered(const double* data, size_t size);

private slots:
  void onReadyRead();

private:
  void processPacket();

  void handleSubscribeRange(const QByteArray& payload);
  void handleSubscribeList(const QByteArray& payload);
  void handleUnsubscribeAll();

private:
  QTcpSocket m_socket;

  // subscription state
  bool m_useRange = false;
  TagId m_from = 0;
  TagId m_to = 0;
  std::vector<TagId> m_list;

  // network buffer (reused, no allocations per tick)
  QByteArray m_outBuffer;
  QByteArray m_inBuffer;
};

}

#endif // CLIENTSESSION_H
