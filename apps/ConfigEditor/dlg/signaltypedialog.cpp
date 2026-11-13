
#include "signaltypedialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>

namespace qds
{

SignalTypeDialog::SignalTypeDialog(
  QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle(tr("Новый тип сигнала"));
  setFixedSize(320, 100);

  auto* layout = new QFormLayout(this);

  m_nameEdit = new QLineEdit(this);
  m_nameEdit->setMaxLength(64);

  layout->addRow(
    tr("Название:"), m_nameEdit);

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

QString SignalTypeDialog::typeName() const
{
  return m_nameEdit->text().trimmed();
}

}
