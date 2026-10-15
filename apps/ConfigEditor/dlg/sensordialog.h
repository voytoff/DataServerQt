#pragma once

#include "sensortype.h"

#include <QDialog>

#include <vector>

class QLineEdit;
class QComboBox;

namespace qds
{

class SensorDialog final : public QDialog
{
  Q_OBJECT

public:
  explicit SensorDialog(
    const std::vector<SensorType>& types,
    QWidget* parent = nullptr);

  QString sensorName() const;

  SensorTypeId sensorTypeId() const;

private:
  QLineEdit* m_nameEdit = nullptr;
  QComboBox* m_typeCombo = nullptr;
};

}
