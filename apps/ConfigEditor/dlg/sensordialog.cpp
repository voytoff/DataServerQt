#include "sensordialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

namespace qds
{

SensorDialog::SensorDialog(
  const std::vector<SensorType>& types,
  QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle(tr("Новый датчик"));

  auto* layout = new QFormLayout(this);

  m_nameEdit = new QLineEdit(this);
  m_typeCombo = new QComboBox(this);

  for (const auto& type : types)
  {
    m_typeCombo->addItem(
      QString::fromStdString(type.name),
      type.id.value);
  }

  layout->addRow(
    tr("Название:"),
    m_nameEdit);

  layout->addRow(
    tr("Тип датчика:"),
    m_typeCombo);

  auto* buttons = new QDialogButtonBox(
    QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel,
    this);

  layout->addRow(buttons);

  connect(
    buttons,
    &QDialogButtonBox::accepted,
    this,
    [this]()
    {
      if (m_nameEdit->text().trimmed().isEmpty())
      {
        QMessageBox::warning(
          this,
          tr("Новый датчик"),
          tr("Введите название датчика."));

        m_nameEdit->setFocus();
        return;
      }

      if (m_typeCombo->currentIndex() < 0)
      {
        QMessageBox::warning(
          this,
          tr("Новый датчик"),
          tr("Выберите тип датчика."));
        return;
      }

      accept();
    });

  connect(
    buttons,
    &QDialogButtonBox::rejected,
    this,
    &QDialog::reject);

  m_nameEdit->setFocus();

  resize(360, sizeHint().height());
}

QString SensorDialog::sensorName() const
{
  return m_nameEdit->text().trimmed();
}

SensorTypeId SensorDialog::sensorTypeId() const
{
  return SensorTypeId{
    m_typeCombo->currentData().toUInt()
  };
}

}
