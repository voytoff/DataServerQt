#include "configurationsignalmodel.h"
#include "signaldefinition.h"

#include <vector>
#include <QSize>

namespace qds
{

ConfigurationSignalModel::ConfigurationSignalModel(
  ConfigurationRepository& repository,
  QObject* parent)
  : QAbstractTableModel(parent)
  , m_repository(repository)
{
}

bool ConfigurationSignalModel::load(
  ConfigurationId configuration)
{
  std::vector<SignalDefinition> signalDefs;

  if (!m_repository.signalDefinitions(
        configuration, signalDefs))
  {
    return false;
  }

  beginResetModel();

  m_configuration = configuration;
  m_signals = std::move(signalDefs);

  endResetModel();

  return true;
}

int ConfigurationSignalModel::rowCount(
  const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  return static_cast<int>(m_signals.size());
}

int ConfigurationSignalModel::columnCount(
  const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  return 3;
}

QVariant ConfigurationSignalModel::data(
  const QModelIndex& index,
  int role) const
{
  if (!index.isValid() ||
      index.row() < 0 ||
      index.row() >= rowCount())
  {
    return {};
  }

  const auto& signal =
    m_signals[index.row()];

  if (role != Qt::DisplayRole)
    return {};

  switch (index.column())
  {
  case 0:
    return QString::fromStdString(signal.name);

  case 1:
    switch (signal.kind)
    {
    case SignalKind::Raw:
      return QStringLiteral("Raw");

    case SignalKind::Calculated:
      return QStringLiteral("Calculated");

    default:
      return {};
    }

  case 2:
    return QStringLiteral("%1 Гц").arg(
      static_cast<int>(signal.archiveRate));
  }

  return {};
}

QVariant ConfigurationSignalModel::headerData(
  int section,
  Qt::Orientation orientation,
  int role) const
{
  if (orientation != Qt::Horizontal ||
      role != Qt::DisplayRole)
  {
    return {};
  }

  switch (section)
  {
  case 0:
    return tr("Название");

  case 1:
    return tr("Вид");

  case 2:
    return tr("Архив");
  }

  return {};
}

const SignalDefinition*
ConfigurationSignalModel::signal(
  const QModelIndex& index) const noexcept
{
  if (!index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_signals.size()))
  {
    return nullptr;
  }

  return &m_signals[index.row()];
}

bool ConfigurationSignalModel::updateSignalName(
  const QModelIndex& index,
  const QString& name)
{
  const auto* s = signal(index);

  if (!s)
    return false;

  if (QString::fromStdString(s->name) == name)
    return true;

  if (!m_repository.updateSignalName(
        m_configuration,
        s->id,
        name))
  {
    const auto error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось сохранить настройки сигнала."));

    return false;
  }

  m_signals[index.row()].name =
    name.toStdString();

  const QModelIndex changed =
    this->index(index.row(), 0);

  emit dataChanged(
    changed,
    changed,
    {Qt::DisplayRole});

  return true;
}

bool ConfigurationSignalModel::updateSignalArchiveRate(
  const QModelIndex& index,
  ArchiveRate rate)
{
  const auto* s = signal(index);

  if (!s || !isValidArchiveRate(rate))
    return false;

  if (s->archiveRate == rate)
    return true;

  if (!m_repository.updateSignalArchiveRate(
        m_configuration,
        s->id,
        rate))
  {
    const auto error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось сохранить настройки сигнала."));

    return false;
  }

  m_signals[index.row()].archiveRate = rate;

  const QModelIndex changed =
    this->index(index.row(), 2);

  emit dataChanged(
    changed,
    changed,
    {Qt::DisplayRole});

  return true;
}

}