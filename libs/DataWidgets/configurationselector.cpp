#include "configurationselector.h"

#include <QSignalBlocker>
#include <QSqlRecord>

namespace qds
{

ConfigurationSelector::ConfigurationSelector(
  const QSqlDatabase& database,
  QWidget* parent)
  : QComboBox(parent)
{
  m_model = new QSqlTableModel(
    this,
    database);

  m_model->setTable("configuration");

  connect(
    this,
    &QComboBox::currentIndexChanged,
    this,
    [this](int index)
    {
      if (index < 0)
        return;

      const ConfigurationId id =
        configurationId();

      if (id.value != 0)
        emit configurationChanged(id);
    });

  reload();
}

bool ConfigurationSelector::reload()
{
  const QSignalBlocker blocker(this);

  if (!m_model->select())
    return false;

  const QSqlRecord record =
    m_model->record();

  m_idColumn =
    record.indexOf("id");

  const int nameColumn =
    record.indexOf("name");

  if (m_idColumn < 0 || nameColumn < 0)
    return false;

  setModel(m_model);
  setModelColumn(nameColumn);
  setCurrentIndex(-1);

  return true;
}

ConfigurationId ConfigurationSelector::configurationId() const
{
  const int row = currentIndex();

  if (row < 0 || m_idColumn < 0)
    return ConfigurationId{};

  bool ok = false;

  const auto value =
    m_model->record(row)
      .value(m_idColumn)
      .toUInt(&ok);

  if (!ok)
    return ConfigurationId{};

  return ConfigurationId{value};
}

bool ConfigurationSelector::setConfigurationId(
  ConfigurationId id)
{
  if (m_idColumn < 0)
    return false;

  for (int row = 0;
       row < m_model->rowCount();
       ++row)
  {
    bool ok = false;

    const auto value =
      m_model->record(row)
        .value(m_idColumn)
        .toUInt(&ok);

    if (!ok || value != id.value)
      continue;

    const QSignalBlocker blocker(this);

    setCurrentIndex(row);

    return true;
  }

  return false;
}

}