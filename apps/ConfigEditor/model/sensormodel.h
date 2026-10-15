
#pragma once

#include "sensor.h"
#include "sensortype.h"

#include <QAbstractTableModel>

#include <vector>

namespace qds
{

class ConfigurationRepository;

class SensorModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit SensorModel(
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

  const Sensor* sensor(
    const QModelIndex& index) const noexcept;

  const std::vector<SensorType>& sensorTypes() const noexcept;

  void setSensorTypes(
    std::vector<SensorType> types);

  bool createSensor(
    const QString& name,
    SensorTypeId typeId,
    SensorId& createdId);

  QModelIndex indexOf(
    SensorId id) const;

  bool removeSensor(
    const QModelIndex& index);

  void updateSensor(
    const Sensor& sensor);

private:
  ConfigurationRepository& m_repository;

  std::vector<Sensor> m_sensors;
  std::vector<SensorType> m_sensorTypes;

signals:
  void operationFailed(
    const QString& message);
};

}
