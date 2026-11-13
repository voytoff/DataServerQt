#include "applicationsettingsloader.h"

#include <QSettings>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>

namespace qds
{


bool ApplicationSettingsLoader::load(
  const QCoreApplication& app,
  ApplicationSettings& settings,
  QString& error,
  bool requireServerSettings)
{
  error.clear();

  QCommandLineParser parser;

  parser.setApplicationDescription(
    app.applicationName());

  parser.addHelpOption();

  QCommandLineOption configOption(
    "config",
    "DataServer INI file",
    "file",
    "dataserver.ini");

  parser.addOption(configOption);
  parser.process(app);

  const SettingsSections sections =
    requireServerSettings
      ? SettingsSection::Database |
          SettingsSection::Archive |
          SettingsSection::Log
      : SettingsSection::Database;

  return load(
    parser.value(configOption),
    settings,
    error,
    sections);
}

bool ApplicationSettingsLoader::load(
  const QString& fileName,
  ApplicationSettings& settings,
  QString& error,
  SettingsSections sections,
  bool allowMissingFile)
{
  error.clear();

  if (!QFileInfo::exists(fileName) &&
      !allowMissingFile)
  {
    error =
      "Не найден файл конфигурации:\n" +
      fileName;

    return false;
  }

  QSettings s(
    fileName,
    QSettings::IniFormat);

  ApplicationSettings result;
  result.configFile = fileName;

  // Database

  if (sections.testFlag(SettingsSection::Database))
  {
    DatabaseSettings database;

    s.beginGroup("Database");

    database.plugin =
      s.value("Plugin", database.plugin).toString();

    database.host =
      s.value("Host", database.host).toString();

    bool portOk = false;

    database.port =
      s.value("Port", database.port).toInt(&portOk);

    database.name =
      s.value("Name").toString();

    database.user =
      s.value("User").toString();

    database.password =
      s.value("Password").toString();

    s.endGroup();

    if (!portOk ||
        database.plugin.isEmpty() ||
        database.host.isEmpty() ||
        database.name.isEmpty() ||
        database.user.isEmpty() ||
        database.port < 1 ||
        database.port > 65535)
    {
      error =
        "Неверные настройки доступа к базе данных:\n" +
        fileName;

      return false;
    }

    result.database = std::move(database);
  }

  // Server/ConfigurationId

  if (s.contains("Server/ConfigurationId"))
  {
    bool idOk = false;

    const uint32_t id =
      s.value("Server/ConfigurationId").toUInt(&idOk);

    if (!idOk)
    {
      error =
        "Некорректный ConfigurationId:\n" +
        fileName;

      return false;
    }

    result.configurationId = ConfigurationId{id};
  }

  // Archive

  if (sections.testFlag(SettingsSection::Archive))
  {
    const QString directory =
      s.value("Archive/Directory").toString();

    if (directory.isEmpty())
    {
      error =
        "Некорректный каталог архивирования:\n" +
        fileName;

      return false;
    }

#ifdef Q_OS_WIN
    result.archiveDirectory =
      std::filesystem::path(directory.toStdWString());
#else
    result.archiveDirectory =
      std::filesystem::u8path(
        directory.toUtf8().constData());
#endif
  }

  // Log

  if (sections.testFlag(SettingsSection::Log))
  {
    const QString directory =
      s.value("Log/Directory").toString();

    if (directory.isEmpty())
    {
      error =
        "Некорректный каталог логирования:\n" +
        fileName;

      return false;
    }

#ifdef Q_OS_WIN
    result.logDirectory =
      std::filesystem::path(directory.toStdWString());
#else
    result.logDirectory =
      std::filesystem::u8path(
        directory.toUtf8().constData());
#endif
  }

  // Table

  if (sections.testFlag(SettingsSection::Table))
  {
    s.beginGroup("Table");

    const int decimals =
      s.value(
         "Decimals",
         result.table.decimals).toInt();

    if (decimals < 0 || decimals > 12)
    {
      error = "Некорректное количество знаков после запятой для таблиц";
      return false;
    }

    result.table.decimals = decimals;

    result.table.showUnitsInHeaders =
      s.value(
         "ShowUnitsInHeaders",
         result.table.showUnitsInHeaders).toBool();

    s.endGroup();
  }

  // Chart

  if (sections.testFlag(SettingsSection::Chart))
  {
    s.beginGroup("Chart");

    result.chart.lineWidth =
      std::clamp(
        s.value(
           "LineWidth",
           result.chart.lineWidth).toInt(),
        1,
        3);

    const QString mode =
      s.value(
         "MarkerMode",
         "Interpolated").toString();

    result.chart.markerMode =
      mode == "NearestSample"
        ? ChartMarkerMode::NearestSample
        : ChartMarkerMode::Interpolated;

    const int decimals =
      s.value(
         "Decimals",
         result.chart.decimals).toInt();

    if (decimals < 0 || decimals > 12)
    {
      error = "Некорректное количество знаков после запятой для графиков";
      return false;
    }

    result.chart.decimals = decimals;

    s.endGroup();
  }

  if (s.status() != QSettings::NoError)
  {
    error =
      "Ошибка чтения файла конфигурации:\n" +
      fileName;

    return false;
  }

  settings = std::move(result);

  return true;
}

bool ApplicationSettingsLoader::save(
  const QString& fileName,
  const ApplicationSettings& settings,
  QString& error,
  SettingsSections sections,
  bool allowCreateFile)
{
  error.clear();

  if (fileName.isEmpty())
  {
    error = "Не указан файл конфигурации";
    return false;
  }

  if (!QFileInfo::exists(fileName) &&
      !allowCreateFile)
  {
    error =
      "Не найден файл конфигурации:\n" +
      fileName;

    return false;
  }

  // Проверяем значения до изменения INI.

  if (sections.testFlag(SettingsSection::Database))
  {
    const auto& db = settings.database;

    if (db.plugin.trimmed().isEmpty() ||
        db.host.trimmed().isEmpty() ||
        db.name.trimmed().isEmpty() ||
        db.user.trimmed().isEmpty() ||
        db.port < 1 ||
        db.port > 65535)
    {
      error = "Некорректные настройки базы данных";
      return false;
    }
  }

  if (sections.testFlag(SettingsSection::Archive) &&
      settings.archiveDirectory.empty())
  {
    error = "Не указан каталог архивирования";
    return false;
  }

  if (sections.testFlag(SettingsSection::Log) &&
      settings.logDirectory.empty())
  {
    error = "Не указан каталог журналирования";
    return false;
  }

  if (sections.testFlag(SettingsSection::Table) &&
      (settings.table.decimals < 0 ||
       settings.table.decimals > 12))
  {
    error = "Некорректное количество знаков после запятой";
    return false;
  }

  QSettings s(
    fileName,
    QSettings::IniFormat);

  if (s.status() != QSettings::NoError)
  {
    error =
      "Ошибка открытия файла конфигурации:\n" +
      fileName;

    return false;
  }

  // Database

  if (sections.testFlag(SettingsSection::Database))
  {
    const auto& db = settings.database;

    s.setValue("Database/Plugin", db.plugin);
    s.setValue("Database/Host", db.host);
    s.setValue("Database/Port", db.port);
    s.setValue("Database/Name", db.name);
    s.setValue("Database/User", db.user);
    s.setValue("Database/Password", db.password);
  }

  // Archive

  if (sections.testFlag(SettingsSection::Archive))
  {
#ifdef Q_OS_WIN
    const QString directory =
      QString::fromStdWString(
        settings.archiveDirectory.wstring());
#else
    const auto utf8 =
      settings.archiveDirectory.u8string();

    const QString directory =
      QString::fromUtf8(
        reinterpret_cast<const char*>(utf8.data()),
        static_cast<qsizetype>(utf8.size()));
#endif

    s.setValue("Archive/Directory", directory);
  }

  // Log

  if (sections.testFlag(SettingsSection::Log))
  {
#ifdef Q_OS_WIN
    const QString directory =
      QString::fromStdWString(
        settings.logDirectory.wstring());
#else
    const auto utf8 =
      settings.logDirectory.u8string();

    const QString directory =
      QString::fromUtf8(
        reinterpret_cast<const char*>(utf8.data()),
        static_cast<qsizetype>(utf8.size()));
#endif

    s.setValue("Log/Directory", directory);
  }

  // Table

  if (sections.testFlag(SettingsSection::Table))
  {
    s.beginGroup("Table");

    s.setValue(
      "Decimals",
      settings.table.decimals);

    s.setValue(
      "ShowUnitsInHeaders",
      settings.table.showUnitsInHeaders);

    s.endGroup();
  }

  // Chart

  if (sections.testFlag(SettingsSection::Chart))
  {
    s.beginGroup("Chart");

    s.setValue(
      "LineWidth",
      settings.chart.lineWidth);

    s.setValue(
      "MarkerMode",
      settings.chart.markerMode ==
          ChartMarkerMode::NearestSample
        ? "NearestSample"
        : "Interpolated");

    s.setValue(
      "Decimals",
      settings.chart.decimals);

    s.endGroup();
  }

  s.sync();

  if (s.status() != QSettings::NoError)
  {
    error =
      "Не удалось сохранить файл конфигурации:\n" +
      fileName;

    return false;
  }

  return true;
}

bool ApplicationSettingsLoader::saveConfigurationId(
  const QString& fileName,
  ConfigurationId id,
  const QString& key)
{
  if (id.value == 0)
    return false;

  QSettings settings(
    fileName,
    QSettings::IniFormat);

  settings.setValue(key, id.value);
  settings.sync();

  return settings.status() == QSettings::NoError;
}

}
