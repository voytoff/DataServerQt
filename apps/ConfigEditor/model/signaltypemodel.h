
#pragma once

#include "signaltypeinfo.h"
#include "measurementunit.h"

#include <QAbstractTableModel>

#include <vector>

namespace qds
{

class ConfigurationRepository;

class SignalTypeModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  enum Column
  {
    Id,
    Name,
    Description,
    MeasurementUnit,
    ColumnCount
  };

  explicit SignalTypeModel(
    ConfigurationRepository& repository,
    QObject* parent = nullptr);

  bool load();

  void setMeasurementUnits(
    const std::vector<qds::MeasurementUnit>& units);

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

  const SignalTypeInfo* signalType(
    const QModelIndex& index) const noexcept;

  bool createSignalType(const QString& name);

  bool removeSignalType(const QModelIndex& index);

  const std::vector<SignalTypeInfo>& signalTypes()
    const noexcept;

signals:
  void operationFailed(const QString& message);
  void signalTypesChanged();

private:
  ConfigurationRepository& m_repository;

  std::vector<SignalTypeInfo> m_types;
  std::vector<qds::MeasurementUnit> m_units;
};

}
