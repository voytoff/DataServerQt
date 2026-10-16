#include "calibrationdialog.h"

#include <QApplication>
#include <QStandardItemModel>
#include <QAbstractItemView>

int main(int argc, char* argv[])
{
  using namespace qds;

  QApplication app(argc, argv);

  Calibration calibration;

  calibration.kind = CalibrationKind::Polynomial;
  calibration.name = "Тестовая тарировка";
  calibration.description = "Проверка редактора";

  if (calibration.kind == CalibrationKind::Table)
  {
    calibration.points = {
      {0, 0.0, 0.0},
    };
  }
  else
  {
    calibration.coefficients = {
      0
    };
  }

  CalibrationDialog dialog(calibration, nullptr);

  if (dialog.exec() != QDialog::Accepted)
    return 0;

  const Calibration result =
    dialog.calibration();

  if (calibration.kind == CalibrationKind::Table)
  {
    qDebug() << "Calibration:"
             << result.name
             << result.description
             << "Points:" << result.points.size();

    for (const auto& point : result.points)
      qDebug() << point.index << point.x << point.y;
  }
  else
  {
    qDebug() << "Calibration:"
             << result.name
             << result.description
             << "Coefficients:" << result.coefficients.size();

    for (const auto& point : result.coefficients)
      qDebug() << point;
  }


  return 0;

/*
  QStandardItemModel model(10, 3);

  model.setHorizontalHeaderLabels({
    "X", "Y", "Z"
  });

  qds::BaseTableView view(120);

  view.setModel(&model);
  view.setPasteEnabled(true);

  view.setSelectionBehavior(
    QAbstractItemView::SelectItems);

  view.setSelectionMode(
    QAbstractItemView::ExtendedSelection);

  view.resize(500, 350);
  view.show();

  return app.exec();*/
}