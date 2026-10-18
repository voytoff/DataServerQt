#pragma once

#include "configurationinfo.h"

#include <QDialog>
#include <memory>

namespace Ui
{
class ConfigurationDialog;
}

namespace qds
{

class ConfigurationDialog final : public QDialog
{
  Q_OBJECT

public:
  explicit ConfigurationDialog(
    QWidget* parent = nullptr);

  ~ConfigurationDialog() override;

  void setConfiguration(
    const ConfigurationInfo& configuration);

  ConfigurationInfo configuration() const;

private:
  void accept() override;

private:
  std::unique_ptr<Ui::ConfigurationDialog> m_ui;

  ConfigurationId m_configurationId;
};

}