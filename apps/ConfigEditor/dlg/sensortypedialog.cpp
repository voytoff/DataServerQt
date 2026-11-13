
#include "sensortypedialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>

namespace qds
{

SensorTypeDialog::SensorTypeDialog(
  QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle(tr("Новый тип датчика"));
  setFixedSize(320, 120);

  auto* layout = new QFormLayout(this);

  m_nameEdit = new QLineEdit(this);
  m_nameEdit->setMaxLength(128);

  layout->addRow(
    tr("Название:"), m_nameEdit);

  m_kindCombo = new QComboBox(this);

  m_kindCombo->addItem(
    tr("Табличная"),
    static_cast<int>(CalibrationKind::Table));

  m_kindCombo->addItem(
    tr("Полиномиальная"),
    static_cast<int>(CalibrationKind::Polynomial));

  layout->addRow(
    tr("Вид тарировки:"), m_kindCombo);

  layout->addItem(
    new QSpacerItem(
      0, 0,
      QSizePolicy::Minimum,
      QSizePolicy::Expanding));

  auto* buttons = new QDialogButtonBox(
    QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel,
    this);

  layout->addRow(buttons);

  auto* okButton =
    buttons->button(QDialogButtonBox::Ok);

  okButton->setEnabled(false);

  connect(
    m_nameEdit,
    &QLineEdit::textChanged,
    this,
    [this, okButton]()
    {
      okButton->setEnabled(
        !typeName().isEmpty());
    });

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

  m_nameEdit->setFocus();
}

QString SensorTypeDialog::typeName() const
{
  return m_nameEdit->text().trimmed();
}

CalibrationKind SensorTypeDialog::calibrationKind() const
{
  return static_cast<CalibrationKind>(
    m_kindCombo->currentData().toInt());
}

}
