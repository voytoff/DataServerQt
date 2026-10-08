#include "mainwindow.h"

#include "configurationeditor.h"

namespace qds
{

MainWindow::MainWindow(
  const QSqlDatabase& database,
  QWidget* parent)
  : QMainWindow(parent)
{
  setWindowIcon(
    QIcon(":/images/main.png"));

  m_editor =
    new ConfigurationEditor(
      database,
      this);

  setCentralWidget(m_editor);

  resize(1200, 800);
  setWindowTitle(
    "DataServer Configuration Editor");
}

}