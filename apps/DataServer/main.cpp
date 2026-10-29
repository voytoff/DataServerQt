#include "applicationsettingsloader.h"
#include "databaseconnection.h"
#include "mainwindow.h"
#include "servercontroller.h"
#include "colorschemesettings.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QSettings>
#include <QTranslator>
#include <QLibraryInfo>
#include <QStyleFactory>
#include <QSqlError>
#include <QMessageBox>

static bool showError(
  const QString& message)
{
  qCritical().noquote() << message;

  QMessageBox::critical(
    nullptr,
    QObject::tr("DataServer — ошибка запуска"),
    message);

  return false;
}

static bool ensureDirectory(
  const std::filesystem::path& directory)
{
  std::error_code ec;

  std::filesystem::create_directories(
    directory, ec);

  if (ec)
  {
    return showError(
      "Не удалось создать каталог:\n" +
      QString::fromStdWString(
        directory.wstring()) +
      "\n\n" +
      QString::fromStdString(
        ec.message()));
  }

  return true;
}


int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

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
        "dataserver_" + lang,
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
        error,
        true))
  {
    showError(error);
    return 1;
  }

  if (!ensureDirectory(settings.archiveDirectory) ||
      !ensureDirectory(settings.logDirectory))
  {
    return 1;
  }

  QSqlDatabase database;

  if (!qds::DatabaseConnection::open(
        settings.database,
        database,
        error))
  {
    return showError(error);
  }

  qds::ColorSchemeSettings::restore(
    settings.configFile);

  qds::ServerController controller(
    database,
    settings.configurationId,
    settings.archiveDirectory,
    settings.logDirectory);

  qds::MainWindow window(
    database,
    controller,
    settings.configFile);

  window.show();

  const int result = app.exec();

  controller.stop();

  return result;
}
