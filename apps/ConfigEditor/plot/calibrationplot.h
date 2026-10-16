#pragma once

#include "calibration.h"

#include <QWidget>

class QCustomPlot;

namespace qds
{

class CalibrationPlot : public QWidget
{
  Q_OBJECT

public:
  explicit CalibrationPlot(
    QWidget* parent = nullptr);

  void setCalibration(
    const Calibration& calibration);

private:
  void updateTable(
    const Calibration& calibration);

  void updatePolynomial(
    const Calibration& calibration);

  void clear();

private:
  QCustomPlot* m_plot = nullptr;
};

}