#include "mainwindow.h"
#include "configurationeditor.h"
#include <QStatusBar>

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

  connect(
    m_editor,
    &ConfigurationEditor::statusMessage,
    this,
    [this](const QString& message)
    {
      statusBar()->showMessage(
        message, 3000);
    });

  resize(1200, 800);
  setWindowTitle(
    "DataServer Configuration Editor");

  statusBar()->setSizeGripEnabled(true);
}

}