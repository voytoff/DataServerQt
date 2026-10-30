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
  return parent.isValid() ? 0 : ColumnCount;
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

  if (role == KindRole)
    return static_cast<int>(signal.kind);

  if (role == Qt::ForegroundRole &&
      index.column() == 0 &&
      signal.kind == SignalKind::Raw)
  {
    const uint32_t tag =
      signal.source.tag.value;

    if (tag != 0 &&
        !m_availableTags.contains(tag))
    {
      return QColor(Qt::red);
    }
  }

  if (role != Qt::DisplayRole)
    return {};

  switch (index.column())
  {
  case Name:
    return QString::fromStdString(signal.name);

  case Source:
    if (signal.kind != SignalKind::Raw)
      return {};

    if (signal.source.tag.value == 0)
      return tr("Нет");

    if (const auto it =
        m_tagNames.find(signal.source.tag.value);
        it != m_tagNames.end())
    {
      return it->second;
    }

    return tr("TagId=%1")
      .arg(signal.source.tag.value);

  case Formula:
    if (signal.kind == SignalKind::Calculated)
      return QString::fromStdString(signal.formula);

    return {};

  case SignalType:
    if (signal.signalType.value == 0)
      return tr("Нет");

    if (const auto it =
        m_signalTypeNames.find(signal.signalType.value);
        it != m_signalTypeNames.end())
    {
      return it->second;
    }

    return tr("Тип #%1")
      .arg(signal.signalType.value);

  case Sensor:
    if (signal.kind != SignalKind::Calculated)
      return {};

    if (signal.sensorId.value == 0)
      return tr("Нет");

    if (const auto it =
        m_sensorNames.find(signal.sensorId.value);
        it != m_sensorNames.end())
    {
      return it->second;
    }

    return tr("Датчик #%1")
      .arg(signal.sensorId.value);

  case Archive:
    return tr("%1 Гц")
      .arg(static_cast<int>(signal.archiveRate));
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
  case Name:
    return tr("Название");

  case Source:
    return tr("Источник");

  case Formula:
    return tr("Формула");

  case SignalType:
    return tr("Тип сигнала");

  case Sensor:
    return tr("Датчик");

  case Archive:
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
    this->index(index.row(), Archive);

  emit dataChanged(
    changed,
    changed,
    {Qt::DisplayRole});

  return true;
}

bool ConfigurationSignalModel::updateSignalSensor(
  const QModelIndex& index,
  SensorId sensorId)
{
  const auto* s = signal(index);

  if (!s || s->kind != SignalKind::Calculated)
    return false;

  if (s->sensorId == sensorId)
    return true;

  if (!m_repository.updateSignalSensor(
        m_configuration,
        s->id,
        sensorId))
  {
    const auto error = m_repository.lastError();

    emit operationFailed(
      error.isValid()
        ? error.text()
        : tr("Не удалось сохранить датчик сигнала."));

    return false;
  }

  m_signals[index.row()].sensorId = sensorId;

  const QModelIndex changed =
    this->index(index.row(), Sensor);

  emit dataChanged(
    changed,
    changed,
    {Qt::DisplayRole});

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

  const QModelIndex changed =
    this->index(index.row(), SignalType);

  emit dataChanged(
    changed,
    changed,
    {Qt::DisplayRole});

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
  
  emit rawSignalSourcesChanged();

  emit dataChanged(
    this->index(index.row(), Name),
    this->index(index.row(), Source),
    {Qt::DisplayRole, Qt::ForegroundRole});

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

  const QModelIndex changed =
    this->index(index.row(), Formula);

  emit dataChanged(
    changed,
    changed,
    {Qt::DisplayRole});

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
  signal.archiveRate = ArchiveRate::Hz10;

  const int row =
    static_cast<int>(m_signals.size());

  beginInsertRows(QModelIndex(), row, row);

  m_signals.push_back(std::move(signal));

  endInsertRows();

  if (kind == SignalKind::Raw)
    emit rawSignalSourcesChanged();

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

  const bool isRaw =
    s->kind == SignalKind::Raw;

  const int row = index.row();

  beginRemoveRows(QModelIndex(), row, row);

  m_signals.erase(m_signals.begin() + row);

  endRemoveRows();

  if (isRaw)
    emit rawSignalSourcesChanged();

  return true;
}

void ConfigurationSignalModel::setTagSources(
  const std::vector<TagSourceInfo>& sources)
{
  std::unordered_set<uint32_t> tags;
  std::unordered_map<uint32_t, QString> names;

  for (const auto& source : sources)
  {
    tags.insert(source.id.value);

    names.emplace(
      source.id.value,
      QString::fromStdString(source.name));
  }

  if (tags == m_availableTags &&
      names == m_tagNames)
  {
    return;
  }

  m_availableTags = std::move(tags);
  m_tagNames = std::move(names);

  if (!m_signals.empty())
  {
    emit dataChanged(
      index(0, Name),
      index(rowCount() - 1, Source),
      {Qt::DisplayRole, Qt::ForegroundRole});
  }
}

void ConfigurationSignalModel::setSignalTypes(
  const std::vector<SignalTypeInfo>& types)
{
  m_signalTypeNames.clear();

  for (const auto& type : types)
  {
    m_signalTypeNames.emplace(
      type.id.value,
      QString::fromStdString(type.name));
  }

  if (!m_signals.empty())
  {
    emit dataChanged(
      index(0, SignalType),
      index(rowCount() - 1, SignalType),
      {Qt::DisplayRole});
  }
}

void ConfigurationSignalModel::setSensors(
  const std::vector<qds::Sensor>& sensors)
{
  m_sensorNames.clear();

  for (const auto& sensor : sensors)
  {
    m_sensorNames.emplace(
      sensor.id.value,
      QString::fromStdString(sensor.name));
  }

  if (!m_signals.empty())
  {
    emit dataChanged(
      index(0, Sensor),
      index(rowCount() - 1, Sensor),
      {Qt::DisplayRole});
  }
}

}
