#include "dataserver.h"
#include "dataengine.h"
#include "livescheduler.h"
#include "livestorage.h"
#include "moduledatasource.h"

#include <QTimer>

DataServer::DataServer(const qds::SystemConfiguration &config, QObject *parent)
  : QObject{parent}
  , m_config(config)
  , m_storage(config)
  , m_scheduler(m_storage, m_subscritions, m_publisher, m_sender)
  , m_server(config, m_subscritions, m_scheduler)
  , m_engine(m_source, m_scheduler)
{
  auto ptr = std::make_unique<qds::ModuleDataSource>(
    m_storage,
    m_config,
    m_config.modules()[0].id,
    m_device,
    m_clock);

  m_source.add(std::move(ptr));
}

bool DataServer::start()
{
  if (!m_engine.start())
    return false;

  QTimer timer;

  connect(
    &timer,
    &QTimer::timeout,
    [&]()
    {
      if (!m_engine.step()) {
        m_engine.stop();
        QCoreApplication::exit(-1);
      }
    });

  timer.start(1);
  return true;
}

void DataServer::stop()
{
  m_engine.stop();
}
