
#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;

namespace qds
{

class MeasurementUnitDialog : public QDialog
{
  Q_OBJECT

public:
  explicit MeasurementUnitDialog(
    QWidget* parent = nullptr);

  QString unitName() const;
  QString unitSymbol() const;

private:
  QLineEdit* m_nameEdit = nullptr;
  QLineEdit* m_symbolEdit = nullptr;
};

}
