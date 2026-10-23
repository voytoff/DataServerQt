#include "mainwindow.h"

namespace qds
{

MainWindow::MainWindow(
  const QSqlDatabase& database,
  const QString& configFile,
  ConfigurationId configurationId,
  QWidget* parent)
  : QMainWindow(parent)
  , m_database(database)
  , m_configFile(configFile)
  , m_configurationId(configurationId)
{
  setWindowTitle("DataMonitor");
  resize(1100, 700);
}

}