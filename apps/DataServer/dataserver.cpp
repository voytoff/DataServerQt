#include "dataserver.h"
#include "dataengine.h"
#include "livescheduler.h"
#include "livestorage.h"
#include "moduleloader.h"
#include <qdebug.h>

DataServer::DataServer(qds::SystemConfiguration cfg, QObject *parent)
  : QObject{parent}
  , m_cfg(std::move(cfg))
  , m_storage(m_cfg)
  , m_scheduler(m_storage, m_subscritions, m_publisher, m_sender)
  , m_server(m_cfg, m_subscritions, m_scheduler)
  , m_engine(m_manager, m_scheduler)
{
  qds::ModuleLoader loader(
    m_cfg,
    m_storage,
    m_clock,
    m_factory,
    m_manager);

  if (!loader.load())
    throw std::runtime_error("Cannot load hardware modules");

  connect(
    &m_timer,
    &QTimer::timeout,
    this,
    &DataServer::onTimer);
}

bool DataServer::start()
{
  if (!m_engine.start())
    return false;

  m_timer.start(1);
  return true;
}

void DataServer::stop()
{
  m_timer.stop();
  m_engine.stop();
}

void DataServer::onTimer()
{
  if (!m_engine.step()) {
    m_engine.stop();
    QCoreApplication::exit(-1);
  }

  qds::Sample sample0, sample1, sample2;
  if (!m_storage.read({0}, sample0)) return;
  if (!m_storage.read({1}, sample1)) return;
  if (!m_storage.read({2}, sample2)) return;

  qDebug() << "вывод" << sample0.value << sample1.value << sample2.value;
}
