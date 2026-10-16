#pragma once

#include "calibration.h"
#include "calibrationtablemodel.h"

#include <QDialog>

class QLineEdit;
class QTextEdit;
class QPushButton;
class QDialogButtonBox;

namespace qds
{

class BaseTableView;
class CalibrationPlot;

class CalibrationDialog final : public QDialog
{
  Q_OBJECT

public:
  explicit CalibrationDialog(
    const Calibration& calibration,
    QWidget* parent = nullptr);

  Calibration calibration() const;

private:
  void addRow();
  void removeRows();
  void updatePlot();
  void accept() override;

private:
  CalibrationTableModel* m_model = nullptr;
  BaseTableView* m_view = nullptr;
  CalibrationPlot* m_plot = nullptr;

  QLineEdit* m_nameEdit = nullptr;
  QTextEdit* m_descriptionEdit = nullptr;

  QPushButton* m_addButton = nullptr;
  QPushButton* m_removeButton = nullptr;
  QDialogButtonBox* m_buttons = nullptr;

  QTimer* m_plotTimer = nullptr;
};

}