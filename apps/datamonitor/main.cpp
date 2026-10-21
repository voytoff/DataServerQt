#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStyleFactory>
#include <QTranslator>

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  QCommandLineParser parser;
  parser.setApplicationDescription(
    "DataServer Configuration Editor");

  parser.addHelpOption();

  QCommandLineOption configOption(
    "config",
    "Configuration INI file",
    "file",
    "dataserver.ini");

  parser.addOption(configOption);
  parser.process(app);

  const QString configFile =
    parser.value(configOption);

  if (!QFileInfo::exists(configFile))
  {
    QMessageBox::critical(
      nullptr,
      "Configuration error",
      "Configuration file not found:\n" +
        configFile);

    return 1;
  }

  QSettings settings(
    configFile,
    QSettings::IniFormat);

  settings.beginGroup("Database");

  const QString plugin =
    settings.value(
              "Plugin",
              "QMARIADB").toString();

  const QString host =
    settings.value(
              "Host",
              "127.0.0.1").toString();

  const int port =
    settings.value(
              "Port",
              3306).toInt();

  const QString databaseName =
    settings.value(
              "Name").toString();

  const QString userName =
    settings.value(
              "User").toString();

  const QString password =
    settings.value(
              "Password").toString();

  settings.endGroup();

  if (settings.status() != QSettings::NoError ||
      databaseName.isEmpty() ||
      userName.isEmpty() ||
      port < 1 ||
      port > 65535)
  {
    QMessageBox::critical(
      nullptr,
      "Configuration error",
      "Invalid database settings:\n" +
        configFile);

    return 1;
  }

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
        "acdanalizer_" + lang,
        ":/translations"))
  {
    app.installTranslator(&myTranslator);
  }

  app.setStyle(
    QStyleFactory::create("windows11"));

  QSqlDatabase database =
    QSqlDatabase::addDatabase(plugin);

  database.setHostName(host);
  database.setPort(port);
  database.setDatabaseName(databaseName);
  database.setUserName(userName);
  database.setPassword(password);

  if (!database.open())
  {
    QMessageBox::critical(
      nullptr,
      "Database error",
      database.lastError().text());

    return 1;
  }

  qds::MainWindow window;
  window.show();

  return app.exec();
}