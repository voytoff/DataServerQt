#include "configurationsignaldialog.h"

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>

namespace qds
{

ConfigurationSignalDialog::ConfigurationSignalDialog(
  QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle(tr("Добавить сигнал"));
  setModal(true);

  m_nameEdit = new QLineEdit(this);
  m_nameEdit->setMaxLength(128);
  m_nameEdit->setPlaceholderText(tr("Название сигнала"));

  m_kindCombo = new QComboBox(this);

  m_kindCombo->addItem(
    QStringLiteral("Raw"),
    static_cast<int>(SignalKind::Raw));

  m_kindCombo->addItem(
    QStringLiteral("Calculated"),
    static_cast<int>(SignalKind::Calculated));

  auto* form = new QFormLayout;

  form->addRow(
    tr("Название:"),
    m_nameEdit);

  form->addRow(
    tr("Вид:"),
    m_kindCombo);

  auto* buttons = new QDialogButtonBox(
    QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel,
    this);

  m_okButton = buttons->button(
    QDialogButtonBox::Ok);

  m_okButton->setText(tr("Добавить"));

  buttons->button(
           QDialogButtonBox::Cancel)->setText(
      tr("Отмена"));

  auto* layout = new QVBoxLayout(this);

  layout->addLayout(form);
  layout->addWidget(buttons);

  connect(
    buttons,
    &QDialogButtonBox::accepted,
    this,
    &QDialog::accept);

  connect(
    buttons,
    &QDialogButtonBox::rejected,
    this,
    &QDialog::reject);

  connect(
    m_nameEdit,
    &QLineEdit::textChanged,
    this,
    [this]()
    {
      updateActions();
    });

  updateActions();

  resize(360, sizeHint().height());

  m_nameEdit->setFocus();
}

QString ConfigurationSignalDialog::signalName() const
{
  return m_nameEdit->text().trimmed();
}

SignalKind ConfigurationSignalDialog::signalKind() const
{
  return static_cast<SignalKind>(
    m_kindCombo->currentData().toInt());
}

void ConfigurationSignalDialog::updateActions()
{
  m_okButton->setEnabled(
    !m_nameEdit->text().trimmed().isEmpty());
}

}