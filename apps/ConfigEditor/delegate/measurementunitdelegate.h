
#pragma once

#include "measurementunitmodel.h"

#include <QComboBox>
#include <QStyledItemDelegate>

namespace qds
{

class MeasurementUnitDelegate : public QStyledItemDelegate
{
public:
  explicit MeasurementUnitDelegate(
    const MeasurementUnitModel& units,
    QObject* parent = nullptr);

  QWidget* createEditor(
    QWidget* parent,
    const QStyleOptionViewItem&,
    const QModelIndex&) const override;

  void setEditorData(
    QWidget* editor,
    const QModelIndex& index) const override;

  void setModelData(
    QWidget* editor,
    QAbstractItemModel* model,
    const QModelIndex& index) const override;

private:
  const MeasurementUnitModel& m_units;
};

}
