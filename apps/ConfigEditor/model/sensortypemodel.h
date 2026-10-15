#pragma once

#include "sensortype.h"

#include <QAbstractTableModel>

#include <vector>

namespace qds
{

class ConfigurationRepository;

class SensorTypeModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit SensorTypeModel(
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

  const SensorType* sensorType(
    const QModelIndex& index) const noexcept;

  bool createSensorType(
    const QString& name);

  bool removeSensorType(
    const QModelIndex& index);

  const std::vector<SensorType>& sensorTypes() const noexcept;

private:
  ConfigurationRepository& m_repository;
  std::vector<SensorType> m_types;

signals:
  void operationFailed(
    const QString& message);

  void sensorTypesChanged();
};

}
