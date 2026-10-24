#pragma once

#include "datatypes.h"
#include "signaldefinition.h"

#include <QAbstractTableModel>
#include <unordered_set>
#include <vector>
#include <strongidhash.h>

namespace qds
{

class SignalSelectionModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit SignalSelectionModel(
    QObject* parent = nullptr);

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

public:
  void setSelectedSignals(
    const std::vector<SignalId>& ids);

  std::vector<SignalId> selectedSignals() const;

  void setSignals(
    const std::vector<SignalDefinition>& signalIds);

private:
  std::vector<SignalDefinition> m_signals;
  std::unordered_set<SignalId> m_selected;
};

}
