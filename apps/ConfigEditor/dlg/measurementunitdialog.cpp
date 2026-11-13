
#include "measurementunitdialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

namespace qds
{

MeasurementUnitDialog::MeasurementUnitDialog(
  QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle(tr("Новая единица измерения"));
  setFixedSize(320, 120);

  auto* layout = new QFormLayout(this);

  m_nameEdit = new QLineEdit(this);
  m_symbolEdit = new QLineEdit(this);

  m_nameEdit->setMaxLength(128);
  m_symbolEdit->setMaxLength(32);

  layout->addRow(
    tr("Наименование:"), m_nameEdit);

  layout->addRow(
    tr("Обозначение:"), m_symbolEdit);

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

  auto updateOkButton = [this, okButton]()
  {
    okButton->setEnabled(
      !unitName().isEmpty() &&
      !unitSymbol().isEmpty());
  };

  connect(
    m_nameEdit,
    &QLineEdit::textChanged,
    this,
    updateOkButton);

  connect(
    m_symbolEdit,
    &QLineEdit::textChanged,
    this,
    updateOkButton);

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

QString MeasurementUnitDialog::unitName() const
{
  return m_nameEdit->text().trimmed();
}

QString MeasurementUnitDialog::unitSymbol() const
{
  return m_symbolEdit->text().trimmed();
}

}
