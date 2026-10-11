#pragma once

#include "db/configurationrepository.h"

#include <QObject>
#include <QAbstractTableModel>

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

  [[nodiscard]]
  bool updateSignalName(
    const QModelIndex& index,
    const QString& name);

  [[nodiscard]]
  bool updateSignalArchiveRate(
    const QModelIndex& index,
    ArchiveRate rate);

private:
  ConfigurationRepository& m_repository;
  ConfigurationId m_configuration;
  std::vector<SignalDefinition> m_signals;

signals:
  void operationFailed(
    const QString& message);

};

}