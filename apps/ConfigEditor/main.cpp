#include "applicationinitializer.h"
#include "mainwindow.h"
#include "colorschemesettings.h"

#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QStyleFactory>
#include <QTranslator>

static bool showError(
  const QString& message)
{
  qCritical().noquote() << message;

  QMessageBox::critical(
    nullptr,
    QObject::tr("ConfigEditor — ошибка запуска"),
    message);

  return false;
}

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  app.setApplicationName("ConfigEditor");

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
  QSqlDatabase database;

  if (!qds::initializeApplication(
        app,
        settings,
        database,
        qds::SettingsSection::Database))
  {
    return 0;
  }

  qds::ColorSchemeSettings::restore(
    settings.configFile);

  qds::MainWindow window(
    database,
    settings.configFile,
    settings.configurationId);

  window.show();

  return app.exec();
}