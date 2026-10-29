#pragma once

#include "datawidgets_export.h"

#include "applicationsettings.h"
#include "settingssection.h"

#include <QDialog>

#include <memory>

namespace Ui
{
class ApplicationSettingsDialog;
}

namespace qds
{

class DATAWIDGETS_EXPORT ApplicationSettingsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ApplicationSettingsDialog(
    const ApplicationSettings& settings,
    SettingsSections sections,
    QWidget* parent = nullptr);

  ~ApplicationSettingsDialog() override;

  void apply();
  void accept() override;

private slots:
  void browseArchiveDirectory();
  void browseLogDirectory();
  void testDatabaseConnection();

private:
  void loadSettings();
  bool saveSettings();
  ApplicationSettings editedSettings() const;
  SettingsSections changedSections(
    const ApplicationSettings& settings) const;


  std::unique_ptr<Ui::ApplicationSettingsDialog> ui;

  ApplicationSettings m_settings;
  SettingsSections m_sections;

signals:
  void settingsApplied(qds::SettingsSections sections);

};

}