
#pragma once

#include <QDialog>
#include <QString>

#include "calibrationkind.h"

class QLineEdit;
class QComboBox;

namespace qds
{

class SensorTypeDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SensorTypeDialog(
    QWidget* parent = nullptr);

  QString typeName() const;
  CalibrationKind calibrationKind() const;

private:
  QLineEdit* m_nameEdit = nullptr;
  QComboBox* m_kindCombo = nullptr;
};

}
