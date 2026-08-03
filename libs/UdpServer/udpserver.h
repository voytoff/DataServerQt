#pragma once

#include <QObject>
#include <QUdpSocket>

#include "packetdispatcher.h"

namespace qds
{

class UdpServer : public QObject
{
  Q_OBJECT

public:
  // Конструктор
  explicit UdpServer(
    PacketDispatcher& dispatcher,
    QObject* parent = nullptr);

  // Запуск
  bool start(uint16_t port);
  // Остановка
  void stop();

  uint16_t port() const noexcept;
  bool isRunning() const noexcept;

private slots:
  // Приём датаграмм
  void onReadyRead();

private:
  QUdpSocket m_socket;
  PacketDispatcher& m_dispatcher;

};

}