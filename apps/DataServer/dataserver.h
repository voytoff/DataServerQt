#ifndef DATASERVER_H
#define DATASERVER_H

#include "dataengine.h"
#include "hardwaremodulefactory.h"
#include "systemclock.h"
#include "udpserver.h"
#include <QObject>
#include <QCoreApplication>
#include <QTimer>

class DataServer : public QObject
{
  Q_OBJECT
public:
  explicit DataServer(qds::SystemConfiguration cfg, QObject* parent = nullptr);
  ~DataServer() = default;

  bool start();
  void stop();

private:
  qds::SystemConfiguration m_cfg;
  qds::DataEngine m_engine;
  qds::DataSourceManager m_manager;
  qds::SubscriptionManager m_subscritions;
  qds::Publisher m_publisher;
  qds::LiveStorage m_storage;
  qds::LiveScheduler m_scheduler;
  qds::TestSender m_sender;
  qds::UdpServer m_server;
  qds::SystemClock m_clock;
  qds::HardwareModuleFactory m_factory;

  QTimer m_timer;

private slots:
  void onTimer();

};

#endif // DATASERVER_H
