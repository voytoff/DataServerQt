#ifndef DATASERVER_H
#define DATASERVER_H

#include "dataengine.h"
#include "fakelcardmodule.h"
#include "systemclock.h"
#include "udpserver.h"
#include <QObject>
#include <QCoreApplication>

class DataServer : public QObject
{
  Q_OBJECT
public:
  explicit DataServer(const qds::SystemConfiguration &config, QObject* parent = nullptr);
  ~DataServer() = default;

  bool start();
  void stop();

private:
  const qds::SystemConfiguration &m_config;
  qds::DataEngine m_engine;
  qds::DataSourceManager m_source;
  qds::SubscriptionManager m_subscritions;
  qds::Publisher m_publisher;
  qds::LiveStorage m_storage;
  qds::TestSender m_sender;
  qds::LiveScheduler m_scheduler;
  qds::UdpServer m_server;
  qds::SystemClock m_clock;
  qds::FakeLCardModule m_device;

private slots:

};

#endif // DATASERVER_H
