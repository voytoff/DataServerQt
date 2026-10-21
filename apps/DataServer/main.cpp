#include "mainwindow.h"
#include "servercontroller.h"

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

static bool getSettings(
  const QCoreApplication& app,
  QSqlDatabase& database,
  qds::ConfigurationId& configuration,
  std::filesystem::path& archiveDirectory,
  std::filesystem::path& logDirectory)
{
  QCommandLineParser parser;
  parser.setApplicationDescription(
    "DataServer Service");

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
    return showError(
      "Не найден файл конфигурации:\n" +
      configFile);
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
    qCritical()
      << "Configuration error"
      << "Invalid database settings:\n"
      << configFile;

    return false;
  }

  settings.beginGroup("Server");

  bool idOk = false;

  const auto configurationId =
    settings.value("ConfigurationId")
      .toUInt(&idOk);

  settings.endGroup();

  if (!idOk || configurationId == 0)
  {
    return showError(
      "Некорректный ConfigurationId");
  }

  settings.beginGroup("Archive");

  const auto archive =
    settings.value("Directory")
      .toString();

  settings.endGroup();

  if (archive.isEmpty())
  {
    return showError(
      "Некорректный каталог архивирования");
  }

  settings.beginGroup("Log");

  const auto log =
    settings.value("Directory")
      .toString();

  settings.endGroup();

  if (log.isEmpty())
  {
    return showError(
      "Некорректный каталог логирования");
  }


  QSqlDatabase db =
    QSqlDatabase::addDatabase(plugin);

  db.setHostName(host);
  db.setPort(port);
  db.setDatabaseName(databaseName);
  db.setUserName(userName);
  db.setPassword(password);

  if (!db.open())
  {
    return showError(
      "Не удалось подключиться к базе данных:\n" +
      db.lastError().text());
  }

  archiveDirectory = std::filesystem::path(archive.toStdString());
  logDirectory = std::filesystem::path(log.toStdString());
  configuration = qds::ConfigurationId{configurationId};

  database = std::move(db);

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

  QSqlDatabase database;
  qds::ConfigurationId configuration;
  std::filesystem::path archivePath;
  std::filesystem::path logPath;

  if (!getSettings(
        app,
        database,
        configuration,
        archivePath,
        logPath))

    return 1;

  if (!ensureDirectory(archivePath) ||
      !ensureDirectory(logPath))
  {
    return 1;
  }

  qds::ServerController controller(
    database,
    configuration,
    archivePath,
    logPath);

  qds::MainWindow window(database, controller);

  window.show();

  const int result = app.exec();

  controller.stop();

  return result;
}
