#pragma once

#include "datatypes.h"
#include <QMainWindow>
#include <qsqldatabase.h>

namespace qds
{

class ConfigurationEditor;

class MainWindow final : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(
    const QSqlDatabase& database,
    const QString& configFile,
    ConfigurationId configurationId,
    QWidget* parent = nullptr);

protected:
  void closeEvent(QCloseEvent* event) override;

private:
  ConfigurationEditor* m_editor = nullptr;
};

}