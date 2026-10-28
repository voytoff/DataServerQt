#pragma once

#include "db/configurationrepository.h"

#include <QObject>
#include <QAbstractTableModel>
#include <QColor>

#include <unordered_set>

namespace qds
{

class ConfigurationSignalModel final
  : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit ConfigurationSignalModel(
    ConfigurationRepository& repository,
    QObject* parent = nullptr);

  [[nodiscard]]
  bool load(ConfigurationId configuration);

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

  [[nodiscard]]
  const SignalDefinition* signal(
    const QModelIndex& index) const noexcept;

  void clear();

  [[nodiscard]]
  bool updateSignalName(
    const QModelIndex& index,
    const QString& name);

  [[nodiscard]]
  bool updateSignalArchiveRate(
    const QModelIndex& index,
    ArchiveRate rate);

  bool updateSignalSensor(
    const QModelIndex& index,
    SensorId sensorId);

  [[nodiscard]]
  bool updateSignalSignalType(
    const QModelIndex& index,
    SignalTypeId type);

  [[nodiscard]]
  bool updateSignalSource(
    const QModelIndex& index,
    TagId tag);

  [[nodiscard]]
  bool updateSignalFormula(
    const QModelIndex& index,
    QString formula);

  [[nodiscard]]
  bool createSignal(
    QString name,
    SignalKind kind);

  [[nodiscard]]
  bool removeSignal(
    const QModelIndex &index);

  void setTagSources(
    const std::vector<TagSourceInfo>& sources);

private:
  ConfigurationRepository& m_repository;
  ConfigurationId m_configuration;
  std::vector<SignalDefinition> m_signals;
  std::unordered_set<uint32_t> m_availableTags;

signals:
  void operationFailed(
    const QString& message);

  void rawSignalSourcesChanged();
};

}