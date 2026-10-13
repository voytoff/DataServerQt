#include "configurationsignalmodel.h"
#include "signaldefinition.h"

#include <strongidhash.h>
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

void ConfigurationSignalModel::clear()
{
  beginResetModel();

  m_signals.clear();
  m_configuration = ConfigurationId{0};

  endResetModel();
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

bool ConfigurationSignalModel::updateSignalCalibrationMode(
  const QModelIndex& index,
  CalibrationMode mode)
{
  const auto* s = signal(index);

  if (!s || !isValidCalibrationMode(mode))
    return false;

  if (s->calibrationMode == mode)
    return true;

  if (!m_repository.updateSignalCalibrationMode(
        m_configuration,
        s->id,
        mode))
  {
    const auto error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось сохранить настройки сигнала."));

    return false;
  }

  m_signals[index.row()].calibrationMode = mode;

  return true;
}

bool ConfigurationSignalModel::updateSignalSignalType(
  const QModelIndex &index,
  SignalTypeId type)
{
  const auto* s = signal(index);

  if (!s)
    return false;

  if (s->signalType == type)
    return true;

  if (!m_repository.updateSignalSignalType(
        m_configuration,
        s->id,
        type))
  {
    const auto error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось сохранить настройки сигнала."));

    return false;
  }

  m_signals[index.row()].signalType = type;

  return true;
}

bool ConfigurationSignalModel::updateSignalSource(
  const QModelIndex& index,
  TagId tag)
{
  const auto* s = signal(index);

  if (!s || s->kind != SignalKind::Raw)
    return false;

  if (s->source.tag == tag)
    return true;

  if (!m_repository.updateSignalSource(
        m_configuration,
        s->id,
        tag))
  {
    const auto error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось сохранить настройки сигнала."));

    return false;
  }

  m_signals[index.row()].source = SignalSource{tag};

  return true;
}

bool ConfigurationSignalModel::updateSignalFormula(
  const QModelIndex &index,
  QString formula)
{
  const auto* s = signal(index);

  if (!s || s->kind != SignalKind::Calculated)
    return false;

  if (QString::fromStdString(s->formula) == formula)
    return true;

  if (!m_repository.updateSignalFormula(
        m_configuration,
        s->id,
        formula))
  {
    const auto error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось сохранить настройки сигнала."));

    return false;
  }

  m_signals[index.row()].formula = formula.toStdString();

  return true;
}

bool ConfigurationSignalModel::createSignal(
  QString name,
  SignalKind kind)
{
  SignalId createdId{0};

  if (!m_repository.createSignal(
        m_configuration,
        name,
        kind,
        createdId))
  {
    const auto error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось создать новый сигнал."));

    return false;
  }

  SignalDefinition signal;

  signal.id = createdId;
  signal.name = name.toStdString();
  signal.kind = kind;

  signal.source = SignalSource{TagId{0}};
  signal.signalType = SignalTypeId{0};
  signal.calibrationMode = CalibrationMode::None;
  signal.archiveRate = ArchiveRate::Hz10;

  const int row =
    static_cast<int>(m_signals.size());

  beginInsertRows(QModelIndex(), row, row);

  m_signals.push_back(std::move(signal));

  endInsertRows();

  return true;
}

bool ConfigurationSignalModel::removeSignal(
  const QModelIndex& index)
{
  const auto* s = signal(index);

  if (!s)
    return false;

  if (!m_repository.removeSignal(
        m_configuration,
        s->id))
  {
    const auto error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось удалить сигнал."));

    return false;
  }

  const int row = index.row();

  beginRemoveRows(QModelIndex(), row, row);

  m_signals.erase(m_signals.begin() + row);

  endRemoveRows();

  return true;
}

}
