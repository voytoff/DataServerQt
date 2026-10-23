#pragma once

#include "datatypes.h"

#include <QMainWindow>
#include <QSqlDatabase>
#include <QString>

namespace qds
{

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(
    const QSqlDatabase& database,
    const QString& configFile,
    ConfigurationId configurationId,
    QWidget* parent = nullptr);

private:
  QSqlDatabase m_database;
  QString m_configFile;
  ConfigurationId m_configurationId;
};

}