#pragma once

#include "datawidgets_export.h"
#include "datatypes.h"

#include <QComboBox>
#include <QSqlDatabase>
#include <QSqlTableModel>

namespace qds
{

class DATAWIDGETS_EXPORT ConfigurationSelector
  : public QComboBox
{
  Q_OBJECT

public:
  explicit ConfigurationSelector(
    const QSqlDatabase& database,
    QWidget* parent = nullptr);

  bool reload();

  ConfigurationId configurationId() const;

  bool setConfigurationId(
    ConfigurationId id);

signals:
  void configurationChanged(
    qds::ConfigurationId id);

private:
  QSqlTableModel* m_model = nullptr;
  int m_idColumn = -1;
};

}