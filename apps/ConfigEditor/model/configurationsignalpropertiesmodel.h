#pragma once

#include "configurationsignalmodel.h"

#include <QAbstractTableModel>
#include <QPersistentModelIndex>
#include <vector>

namespace qds
{

class ConfigurationSignalPropertiesModel final
  : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit ConfigurationSignalPropertiesModel(
    ConfigurationSignalModel* signalModel,
    QObject* parent = nullptr);

  void setItem(
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

  bool setData(
    const QModelIndex& index,
    const QVariant& value,
    int role = Qt::EditRole) override;

  Qt::ItemFlags flags(
    const QModelIndex& index) const override;

  void setTagSources(
    std::vector<TagSourceInfo> sources);

  void setSignalTypes(
    std::vector<SignalTypeInfo> types);

  void setSensors(
    std::vector<Sensor> sensors);

public:
  enum Role
  {
    PropertyIdRole = Qt::UserRole + 1
  };

  enum class PropertyId
  {
    Name,
    Kind,
    Source,
    Formula,
    SignalType,
    ArchiveRate,
    //CalibrationMode,
    Sensor,
    //PreprocessingFormula
  };

private:
  struct Property
  {
    PropertyId id;
    QString name;
    QVariant value;
  };

  ConfigurationSignalModel* m_signalModel = nullptr;

  QPersistentModelIndex m_currentItem;
  std::vector<Property> m_properties;

  std::vector<SignalTypeInfo> m_signalTypes;
  std::vector<TagSourceInfo> m_tagSources;
  std::vector<Sensor> m_sensors;

signals:
  void operationFailed(
    const QString& message);

};

}