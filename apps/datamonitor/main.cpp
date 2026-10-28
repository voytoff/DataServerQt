#include "applicationsettingsloader.h"
#include "databaseconnection.h"
#include "mainwindow.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QStyleFactory>
#include <QTranslator>
#include <QStyleHints>

static bool showError(
  const QString& message)
{
  qCritical().noquote() << message;

  QMessageBox::critical(
    nullptr,
    QObject::tr("DataMonitor — ошибка запуска"),
    message);

  return false;
}

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  app.setApplicationName("DataMonitor");

  QTranslator qtTranslator;

  const QString lang =
    QLocale::system().name();

  if (qtTranslator.load(
        "qt_" + lang,
        QLibraryInfo::path(
          QLibraryInfo::TranslationsPath)))
  {
    app.installTranslator(&qtTranslator);
  }

  QTranslator myTranslator;

  if (myTranslator.load(
        "configeditor_" + lang,
        ":/translations"))
  {
    app.installTranslator(&myTranslator);
  }

  app.setStyle(
    QStyleFactory::create("windows11"));

  qds::ApplicationSettings settings;
  QString error;

  if (!qds::ApplicationSettingsLoader::load(
        app,
        settings,
        error))
  {
    showError(error);
    return 1;
  }

  QSqlDatabase database;

  if (!qds::DatabaseConnection::open(
        settings.database,
        database,
        error))
  {
    showError(error);
    return 1;
  }

  qds::MainWindow window(
    database,
    settings.configFile,
    settings.configurationId);

  window.show();

  return app.exec();
}