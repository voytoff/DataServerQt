
#pragma once

#include "sensormodel.h"

#include <QAbstractTableModel>

namespace qds
{

class SensorPropertiesModel final
  : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit SensorPropertiesModel(
    SensorModel& sensors,
    ConfigurationRepository& repository,
    QObject* parent = nullptr);

  void setSensor(
    const QModelIndex& index);

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

private:
  SensorModel& m_sensors;
  ConfigurationRepository& m_repository;

  SensorId m_sensorId;

signals:
  void operationFailed(
    const QString& message);

};

}
