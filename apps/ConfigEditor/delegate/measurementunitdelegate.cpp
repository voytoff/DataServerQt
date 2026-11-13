#include "measurementunitdelegate.h"

namespace qds
{

MeasurementUnitDelegate::MeasurementUnitDelegate(
  const MeasurementUnitModel &units,
  QObject *parent)
  : QStyledItemDelegate(parent)
  , m_units(units)
{
}

QWidget *MeasurementUnitDelegate::createEditor(
  QWidget *parent,
  const QStyleOptionViewItem &,
  const QModelIndex &) const
{
  auto* combo = new QComboBox(parent);

  combo->setAutoFillBackground(true);
  combo->setBackgroundRole(QPalette::Base);

  combo->addItem(tr("Нет"), 0);

  for (const auto& unit : m_units.measurementUnits())
  {
    const QString symbol =
      QString::fromStdString(unit.symbol);

    const QString name =
      QString::fromStdString(unit.name);

    combo->addItem(
      QString("%1 (%2)").arg(name, symbol),
      unit.id.value);
  }

  return combo;
}

void MeasurementUnitDelegate::setEditorData(
  QWidget *editor,
  const QModelIndex &index) const
{
  auto* combo = static_cast<QComboBox*>(editor);

  const int row =
    combo->findData(index.data(Qt::EditRole));

  combo->setCurrentIndex(row >= 0 ? row : 0);
}

void MeasurementUnitDelegate::setModelData(
  QWidget *editor,
  QAbstractItemModel *model,
  const QModelIndex &index) const
{
  auto* combo = static_cast<QComboBox*>(editor);

  model->setData(
    index,
    combo->currentData(),
    Qt::EditRole);
}

}
