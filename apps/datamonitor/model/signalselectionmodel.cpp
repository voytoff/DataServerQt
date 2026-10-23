#include "signalselectionmodel.h"

namespace qds
{

SignalSelectionModel::SignalSelectionModel(
  QObject* parent)
  : QAbstractTableModel(parent)
{
}

void SignalSelectionModel::setConfiguration(
  const SystemConfiguration& configuration)
{
  beginResetModel();

  m_signals = configuration.signalDefinitions();
  m_selected.clear();

  endResetModel();
}

void SignalSelectionModel::setSelectedSignals(
  const std::vector<SignalId>& ids)
{
  m_selected.clear();

  for (SignalId id : ids)
    m_selected.insert(id);

  if (!m_signals.empty())
  {
    emit dataChanged(
      index(0, 0),
      index(rowCount() - 1, 0),
      {Qt::CheckStateRole});
  }
}

std::vector<SignalId>
SignalSelectionModel::selectedSignals() const
{
  std::vector<SignalId> result;

  for (const auto& signal : m_signals)
  {
    if (m_selected.contains(signal.id))
      result.push_back(signal.id);
  }

  return result;
}

int SignalSelectionModel::rowCount(
  const QModelIndex& parent) const
{
  return parent.isValid()
  ? 0
  : static_cast<int>(m_signals.size());
}

int SignalSelectionModel::columnCount(
  const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 3;
}

QVariant SignalSelectionModel::data(
  const QModelIndex& idx,
  int role) const
{
  if (!idx.isValid() ||
      idx.row() < 0 ||
      idx.row() >= rowCount())
    return {};

  const auto& signal = m_signals[idx.row()];

  if (role == Qt::CheckStateRole &&
      idx.column() == 0)
  {
    return m_selected.contains(signal.id)
    ? Qt::Checked
    : Qt::Unchecked;
  }

  if (role == Qt::DisplayRole)
  {
    switch (idx.column())
    {
    case 1:
      return static_cast<quint32>(signal.id.value);

    case 2:
      return QString::fromStdString(signal.name);
    }
  }

  return {};
}

QVariant SignalSelectionModel::headerData(
  int section,
  Qt::Orientation orientation,
  int role) const
{
  if (orientation != Qt::Horizontal ||
      role != Qt::DisplayRole)
    return {};

  switch (section)
  {
  case 0: return tr("Выбор");
  case 1: return tr("ID");
  case 2: return tr("Сигнал");
  }

  return {};
}

Qt::ItemFlags SignalSelectionModel::flags(
  const QModelIndex& idx) const
{
  if (!idx.isValid())
    return Qt::NoItemFlags;

  auto result =
    Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if (idx.column() == 0)
    result |= Qt::ItemIsUserCheckable;

  return result;
}

bool SignalSelectionModel::setData(
  const QModelIndex& idx,
  const QVariant& value,
  int role)
{
  if (!idx.isValid() ||
      idx.column() != 0 ||
      role != Qt::CheckStateRole)
    return false;

  const auto& signal = m_signals[idx.row()];

  if (value.toInt() == Qt::Checked)
    m_selected.insert(signal.id);
  else
    m_selected.erase(signal.id);

  emit dataChanged(
    idx, idx, {Qt::CheckStateRole});

  return true;
}

}
