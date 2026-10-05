#pragma once

#include <QWidget>

class QSqlTableModel;
class QSqlRelationalTableModel;
class QTableView;
class QSqlDatabase;

namespace qds
{

class ConfigurationEditor final : public QWidget
{
  Q_OBJECT

public:
  explicit ConfigurationEditor(
    const QSqlDatabase& database,
    QWidget* parent = nullptr);

private:
  QTableView* m_configurationsView = nullptr;
  QTableView* m_modulesView = nullptr;
  QTableView* m_tagsView = nullptr;

  QSqlTableModel* m_configurations = nullptr;
  QSqlRelationalTableModel* m_modules = nullptr;
  QSqlRelationalTableModel* m_tags = nullptr;
};

}