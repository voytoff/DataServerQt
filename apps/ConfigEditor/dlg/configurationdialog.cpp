#include "configurationdialog.h"
#include "ui_configurationdialog.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QTextEdit>

namespace qds
{

ConfigurationDialog::ConfigurationDialog(
  QWidget* parent)
  : QDialog(parent)
  , m_ui(std::make_unique<Ui::ConfigurationDialog>())
{
  m_ui->setupUi(this);

  connect(
    m_ui->buttonBox,
    &QDialogButtonBox::accepted,
    this,
    &ConfigurationDialog::accept);

  connect(
    m_ui->buttonBox,
    &QDialogButtonBox::rejected,
    this,
    &QDialog::reject);

  m_ui->editName->setFocus();
}

ConfigurationDialog::~ConfigurationDialog() = default;

void ConfigurationDialog::setConfiguration(
  const ConfigurationInfo& configuration)
{
  m_configurationId = configuration.id;

  m_ui->editName->setText(
    configuration.name);

  m_ui->editDescription->setPlainText(
    configuration.description);

  m_ui->editPort->setValue(
    configuration.udpPort);
}

ConfigurationInfo ConfigurationDialog::configuration() const
{
  ConfigurationInfo result;

  result.id = m_configurationId;

  result.name =
    m_ui->editName->text().trimmed();

  result.description =
    m_ui->editDescription->toPlainText();

  result.udpPort =
    static_cast<uint16_t>(
      m_ui->editPort->value());

  return result;
}

void ConfigurationDialog::accept()
{
  if (configuration().name.isEmpty())
  {
    QMessageBox::warning(
      this,
      tr("Ошибка"),
      tr("Название эксперимента не может быть пустым."));

    m_ui->editName->setFocus();
    return;
  }

  QDialog::accept();
}

}