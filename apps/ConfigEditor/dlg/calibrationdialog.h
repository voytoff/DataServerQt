#pragma once

#include "calibration.h"
#include "calibrationtablemodel.h"

#include <QDialog>

#include <memory>

class QTimer;

namespace Ui
{
class CalibrationDialog;
}

namespace qds
{

class CalibrationDialog final : public QDialog
{
  Q_OBJECT

public:
  explicit CalibrationDialog(
    const Calibration& calibration,
    QWidget* parent = nullptr);

  ~CalibrationDialog() override;

  Calibration calibration() const;

private:
  void addRow();
  void removeRows();
  void updatePlot();
  void accept() override;

private:
  std::unique_ptr<Ui::CalibrationDialog> m_ui;

  CalibrationTableModel* m_model = nullptr;
  QTimer* m_plotTimer = nullptr;
};

}