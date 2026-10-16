#pragma once

#include "calibration.h"

#include <QAbstractTableModel>

namespace qds
{

class CalibrationTableModel final
  : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit CalibrationTableModel(
    QObject* parent = nullptr);

  void setCalibration(
    const Calibration& calibration);

  const Calibration& calibration() const noexcept;

  int rowCount(
    const QModelIndex& parent = {}) const override;

  int columnCount(
    const QModelIndex& parent = {}) const override;

  QVariant data(
    const QModelIndex& index,
    int role = Qt::DisplayRole) const override;

  QVariant headerData(
    int section,
    Qt::Orientation orientation,
    int role = Qt::DisplayRole) const override;

  Qt::ItemFlags flags(
    const QModelIndex& index) const override;

  bool setData(
    const QModelIndex& index,
    const QVariant& value,
    int role = Qt::EditRole) override;

  bool insertRows(
    int row,
    int count,
    const QModelIndex& parent = {}) override;

  bool removeRows(
    int row,
    int count,
    const QModelIndex& parent = {}) override;

private:
  void updatePointIndices();

  Calibration m_calibration;
};

}