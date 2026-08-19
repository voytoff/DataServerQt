#pragma once

#include <QObject>
#include <QTimer>

#include "runtimesystem.h"
#include "systemconfiguration.h"

#include "datasourcefactory.h"
#include "iarchivewriter.h"
#include "iframepublisher.h"
#include "ischedulerclock.h"

namespace qds
{

class DataServer : public QObject
{
  Q_OBJECT

public:

  explicit DataServer(
    SystemConfiguration configuration,
    const DataSourceFactory& dataSourceFactory,
    IArchiveWriter& archive,
    IFramePublisher& publisher,
    ISchedulerClock& clock,
    QObject* parent = nullptr);

  bool start();
  void stop();

private slots:

  void onTimer();

private:

  SystemConfiguration m_configuration;

  const DataSourceFactory& m_dataSourceFactory;

  IArchiveWriter& m_archive;
  IFramePublisher& m_publisher;
  ISchedulerClock& m_clock;

  RuntimeSystem m_runtime;

  QTimer m_timer;

  bool m_running = false;
};

}