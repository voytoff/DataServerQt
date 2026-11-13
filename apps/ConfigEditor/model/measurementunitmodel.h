#pragma once

#include "measurementunit.h"

#include <QAbstractTableModel>

#include <vector>

namespace qds
{

class ConfigurationRepository;

class MeasurementUnitModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit MeasurementUnitModel(
    ConfigurationRepository& repository,
    QObject* parent = nullptr);

  bool load();

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

  const MeasurementUnit* measurementUnit(
    const QModelIndex& index) const noexcept;

  bool createMeasurementUnit(
    const QString& name,
    const QString& symbol);

  bool removeMeasurementUnit(
    const QModelIndex& index);

  const std::vector<MeasurementUnit>& measurementUnits()
    const noexcept;

signals:
  void operationFailed(const QString& message);
  void measurementUnitsChanged();

private:
  ConfigurationRepository& m_repository;
  std::vector<MeasurementUnit> m_units;
};

}