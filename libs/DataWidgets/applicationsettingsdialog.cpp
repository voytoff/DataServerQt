#include "applicationsettingsdialog.h"
#include "applicationsettingsloader.h"
#include "ui_applicationsettingsdialog.h"

#include <QDialogButtonBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QFileDialog>
#include <QIcon>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStyle>
#include <QUuid>
#include <QString>
#include <QComboBox>

#include <filesystem>

namespace qds
{

namespace
{

QString pathToString(const std::filesystem::path& path)
{
#ifdef Q_OS_WIN
  return QString::fromStdWString(path.wstring());
#else
  const auto utf8 = path.u8string();

  return QString::fromUtf8(
    reinterpret_cast<const char*>(utf8.data()),
    utf8.size());
#endif
}

std::filesystem::path stringToPath(const QString& value)
{
#ifdef Q_OS_WIN
  return std::filesystem::path(value.toStdWString());
#else
  const QByteArray utf8 = value.toUtf8();

  return std::filesystem::u8path(
    utf8.constData(),
    utf8.constData() + utf8.size());
#endif
}

}

ApplicationSettingsDialog::ApplicationSettingsDialog(
  const ApplicationSettings& settings,
  SettingsSections sections,
  QWidget* parent)
  : QDialog(parent),
  ui(std::make_unique<Ui::ApplicationSettingsDialog>()),
  m_settings(settings),
  m_sections(sections)
{
  ui->setupUi(this);

  setWindowTitle(tr("Настройки приложения"));

  ui->tabWidget->setDocumentMode(true);

  ui->tabWidget->setTabVisible(
    ui->tabWidget->indexOf(ui->databasePage),
    sections.testFlag(SettingsSection::Database));

  ui->tabWidget->setTabVisible(
    ui->tabWidget->indexOf(ui->archivePage),
    sections.testFlag(SettingsSection::Archive));

  ui->tabWidget->setTabVisible(
    ui->tabWidget->indexOf(ui->logPage),
    sections.testFlag(SettingsSection::Log));

  connect(
    ui->buttonBox,
    &QDialogButtonBox::accepted,
    this,
    &ApplicationSettingsDialog::accept);

  connect(
    ui->buttonBox->button(QDialogButtonBox::Apply),
    &QPushButton::clicked,
    this,
    &ApplicationSettingsDialog::apply);

  connect(
    ui->testConnectionButton,
    &QPushButton::clicked,
    this,
    &ApplicationSettingsDialog::testDatabaseConnection);


  const QIcon folderIcon =
    QIcon::fromTheme(
      "folder-open",
      style()->standardIcon(QStyle::SP_DirOpenIcon));

  auto* archiveAction =
    ui->archiveDirectoryEdit->addAction(
      folderIcon,
      QLineEdit::TrailingPosition);

  connect(
    archiveAction,
    &QAction::triggered,
    this,
    &ApplicationSettingsDialog::browseArchiveDirectory);

  auto* logAction =
    ui->logDirectoryEdit->addAction(
      folderIcon,
      QLineEdit::TrailingPosition);

  connect(
    logAction,
    &QAction::triggered,
    this,
    &ApplicationSettingsDialog::browseLogDirectory);

  ui->pluginComboBox->addItems(
    QSqlDatabase::drivers());

  loadSettings();
}

ApplicationSettingsDialog::~ApplicationSettingsDialog() = default;

void ApplicationSettingsDialog::apply()
{
  saveSettings();
}

void ApplicationSettingsDialog::accept()
{
  if (!saveSettings())
    return;

  QDialog::accept();
}

void ApplicationSettingsDialog::browseArchiveDirectory()
{
  const QString directory =
    QFileDialog::getExistingDirectory(
      this,
      tr("Выберите каталог архива"),
      ui->archiveDirectoryEdit->text());

  if (!directory.isEmpty())
    ui->archiveDirectoryEdit->setText(directory);
}

void ApplicationSettingsDialog::browseLogDirectory()
{
  const QString directory =
    QFileDialog::getExistingDirectory(
      this,
      tr("Выберите каталог журнала"),
      ui->logDirectoryEdit->text());

  if (!directory.isEmpty())
    ui->logDirectoryEdit->setText(directory);
}

void ApplicationSettingsDialog::testDatabaseConnection()
{
  const DatabaseSettings dbSettings =
    editedSettings().database;

  if (dbSettings.plugin.isEmpty() ||
      dbSettings.host.isEmpty() ||
      dbSettings.name.isEmpty() ||
      dbSettings.user.isEmpty())
  {
    QMessageBox::warning(
      this,
      tr("Проверка соединения"),
      tr("Заполните параметры подключения к базе данных."));

    return;
  }

  if (!QSqlDatabase::isDriverAvailable(dbSettings.plugin))
  {
    QMessageBox::warning(
      this,
      tr("Проверка соединения"),
      tr("Драйвер базы данных %1 недоступен.")
        .arg(dbSettings.plugin));

    return;
  }

  const QString connectionName =
    QStringLiteral("ApplicationSettingsDialog_Test_%1")
      .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));

  QString error;

  {
    QSqlDatabase db =
      QSqlDatabase::addDatabase(
        dbSettings.plugin,
        connectionName);

    db.setHostName(dbSettings.host);
    db.setPort(dbSettings.port);
    db.setDatabaseName(dbSettings.name);
    db.setUserName(dbSettings.user);
    db.setPassword(dbSettings.password);

    if (!db.open())
      error = db.lastError().text();

    db.close();
  }

  QSqlDatabase::removeDatabase(connectionName);

  if (!error.isEmpty())
  {
    QMessageBox::warning(
      this,
      tr("Проверка соединения"),
      tr("Не удалось подключиться к базе данных:\n%1")
        .arg(error));

    return;
  }

  QMessageBox::information(
    this,
    tr("Проверка соединения"),
    tr("Соединение с базой данных успешно установлено."));
}

void ApplicationSettingsDialog::loadSettings()
{
  const auto& db = m_settings.database;

  int index =
    ui->pluginComboBox->findText(db.plugin);

  if (index < 0)
  {
    ui->pluginComboBox->addItem(db.plugin);
    index = ui->pluginComboBox->count() - 1;
  }

  ui->pluginComboBox->setCurrentIndex(index);

  ui->hostEdit->setText(db.host);
  ui->portSpinBox->setValue(db.port);
  ui->databaseEdit->setText(db.name);
  ui->userEdit->setText(db.user);
  ui->passwordEdit->setText(db.password);

  ui->archiveDirectoryEdit->setText(
    pathToString(m_settings.archiveDirectory));

  ui->logDirectoryEdit->setText(
    pathToString(m_settings.logDirectory));
}

bool ApplicationSettingsDialog::saveSettings()
{
  const ApplicationSettings edited =
    editedSettings();

  const SettingsSections changed =
    changedSections(edited);

  if (changed == SettingsSections{})
    return true;

  QString error;

  if (!ApplicationSettingsLoader::save(
        m_settings.configFile,
        edited,
        error,
        changed))
  {
    QMessageBox::warning(
      this,
      tr("Ошибка сохранения настроек"),
      error);

    return false;
  }

  m_settings = edited;

  emit settingsApplied(changed);

  return true;
}

ApplicationSettings ApplicationSettingsDialog::editedSettings() const
{
  ApplicationSettings result = m_settings;

  if (m_sections.testFlag(SettingsSection::Database))
  {
    auto& db = result.database;

    db.plugin =
      ui->pluginComboBox->currentText();

    db.host = ui->hostEdit->text().trimmed();
    db.port = ui->portSpinBox->value();
    db.name = ui->databaseEdit->text().trimmed();
    db.user = ui->userEdit->text().trimmed();
    db.password = ui->passwordEdit->text();
  }

  if (m_sections.testFlag(SettingsSection::Archive))
  {
    result.archiveDirectory =
      stringToPath(
        ui->archiveDirectoryEdit->text().trimmed());
  }

  if (m_sections.testFlag(SettingsSection::Log))
  {
    result.logDirectory =
      stringToPath(
        ui->logDirectoryEdit->text().trimmed());
  }

  return result;
}

SettingsSections ApplicationSettingsDialog::changedSections(
  const ApplicationSettings& settings) const
{
  SettingsSections changed;

  if (m_sections.testFlag(SettingsSection::Database))
  {
    const auto& a = m_settings.database;
    const auto& b = settings.database;

    if (a.plugin != b.plugin ||
        a.host != b.host ||
        a.port != b.port ||
        a.name != b.name ||
        a.user != b.user ||
        a.password != b.password)
    {
      changed |= SettingsSection::Database;
    }
  }

  if (m_sections.testFlag(SettingsSection::Archive) &&
      m_settings.archiveDirectory != settings.archiveDirectory)
  {
    changed |= SettingsSection::Archive;
  }

  if (m_sections.testFlag(SettingsSection::Log) &&
      m_settings.logDirectory != settings.logDirectory)
  {
    changed |= SettingsSection::Log;
  }

  return changed;
}

}
