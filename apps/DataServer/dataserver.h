#pragma once

#include <QObject>
#include <QTimer>

#include "packetdispatcher.h"
#include "publisher.h"
#include "runtimesystem.h"
#include "subscriptionmanager.h"
#include "systemconfiguration.h"

#include "datasourcefactory.h"
#include "iarchivewriter.h"
#include "ischedulerclock.h"
#include "isender.h"
#include "udpserver.h"

namespace qds
{

class DataServer : public QObject
{
  Q_OBJECT

public:

  explicit DataServer(
    SystemConfiguration configuration,
    const CalibrationRepository &repository,
    const DataSourceFactory& dataSourceFactory,
    IArchiveWriter& archive,
    ISchedulerClock& clock,
    ISender& sender,
    QObject* parent = nullptr);

  bool start();
  void stop();

  [[nodiscard]]
  bool isRunning() const noexcept;

private slots:

  void cleanup();

  void onTimer();

private:

  SystemConfiguration m_configuration;
  const CalibrationRepository& m_repository;

  const DataSourceFactory& m_dataSourceFactory;

  IArchiveWriter& m_archive;
  ISchedulerClock& m_clock;
  ISender& m_sender;

  RuntimeSystem m_runtime;

  SubscriptionManager m_subscriptions;

  std::unique_ptr<Publisher> m_publisher;
  std::unique_ptr<PacketDispatcher> m_dispatcher;
  std::unique_ptr<UdpServer> m_udpServer;

  QTimer m_timer;

  bool m_running = false;
};

}