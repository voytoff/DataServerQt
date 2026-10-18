#include "calibrationdialog.h"

#include "dlg/ui_calibrationdialog.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

namespace qds
{

CalibrationDialog::CalibrationDialog(
  const Calibration& calibration,
  QWidget* parent)
  : QDialog(parent)
  , m_ui(std::make_unique<Ui::CalibrationDialog>())
{
  m_ui->setupUi(this);
/*
  setWindowFlag(Qt::WindowContextHelpButtonHint, false);
  setWindowFlag(Qt::CustomizeWindowHint, true);
  setWindowFlag(Qt::WindowTitleHint, true);
  setWindowFlag(Qt::WindowCloseButtonHint, true);
  setWindowFlag(Qt::WindowSystemMenuHint, false);
*/
  // Модель тарировки.
  m_model = new CalibrationTableModel(this);
  m_model->setCalibration(calibration);

  // Основные поля.
  m_ui->descriptionEdit->setPlainText(
    calibration.description);

  // Таблица.
  m_ui->tableView->setModel(m_model);

  m_ui->tableView->setPasteEnabled(true);
  m_ui->tableView->setPasteInsertRowsEnabled(true);

  m_ui->tableView->setSelectionBehavior(
    QAbstractItemView::SelectRows);

  m_ui->tableView->setSelectionMode(
    QAbstractItemView::ExtendedSelection);

  m_ui->tableView->horizontalHeader()
    ->setStretchLastSection(true);

  // Разделитель таблицы и графика.
  m_ui->splitter->setStretchFactor(0, 1);
  m_ui->splitter->setStretchFactor(1, 1);
  m_ui->splitter->setSizes({250, 350});

  // Отложенное обновление графика.
  m_plotTimer = new QTimer(this);
  m_plotTimer->setSingleShot(true);
  m_plotTimer->setInterval(0);


  connect(
    m_plotTimer,
    &QTimer::timeout,
    this,
    &CalibrationDialog::updatePlot);

  // Изменение данных модели.
  auto schedulePlotUpdate = [this]()
  {
    m_plotTimer->start();
  };

  connect(
    m_model,
    &QAbstractItemModel::dataChanged,
    this,
    schedulePlotUpdate);

  connect(
    m_model,
    &QAbstractItemModel::rowsInserted,
    this,
    schedulePlotUpdate);

  connect(
    m_model,
    &QAbstractItemModel::rowsRemoved,
    this,
    schedulePlotUpdate);

  connect(
    m_model,
    &QAbstractItemModel::modelReset,
    this,
    schedulePlotUpdate);

  // Кнопки управления строками.
  connect(
    m_ui->addButton,
    &QPushButton::clicked,
    this,
    &CalibrationDialog::addRow);

  connect(
    m_ui->removeButton,
    &QPushButton::clicked,
    this,
    &CalibrationDialog::removeRows);

  // OK / Cancel.
  connect(
    m_ui->buttonBox,
    &QDialogButtonBox::accepted,
    this,
    &CalibrationDialog::accept);

  connect(
    m_ui->buttonBox,
    &QDialogButtonBox::rejected,
    this,
    &QDialog::reject);

  updatePlot();

  const int column =
    calibration.kind == CalibrationKind::Polynomial
      ? 1
      : 0;

  if (m_model->rowCount() > 0)
  {
    m_ui->tableView->setCurrentIndex(
      m_model->index(0, column));
  }

  m_ui->tableView->setFocus();
}

CalibrationDialog::~CalibrationDialog() = default;

Calibration CalibrationDialog::calibration() const
{
  Calibration result = m_model->calibration();

  result.description =
    m_ui->descriptionEdit->toPlainText();

  return result;
}

void CalibrationDialog::addRow()
{
  const QModelIndex current =
    m_ui->tableView->currentIndex();

  const int row =
    current.isValid()
      ? current.row() + 1
      : m_model->rowCount();

  if (!m_model->insertRow(row))
    return;

  m_ui->tableView->selectRow(row);

  const int column =
    m_model->calibration().kind ==
        CalibrationKind::Polynomial
      ? 1
      : 0;

  const QModelIndex index =
    m_model->index(row, column);

  m_ui->tableView->setCurrentIndex(index);
  m_ui->tableView->edit(index);
}

void CalibrationDialog::removeRows()
{
  const auto selected =
    m_ui->tableView->selectionModel()
      ->selectedRows();

  if (selected.isEmpty())
    return;

  std::vector<int> rows;
  rows.reserve(selected.size());

  for (const auto& index : selected)
    rows.push_back(index.row());

  std::sort(
    rows.begin(),
    rows.end(),
    std::greater<int>());

  for (const int row : rows)
    m_model->removeRow(row);
}

void CalibrationDialog::updatePlot()
{
  m_ui->plotWidget->setCalibration(
    m_model->calibration());
}

void CalibrationDialog::accept()
{
  Calibration result = calibration();

  if (result.kind == CalibrationKind::Table)
  {
    if (!result.buildSegments())
    {
      QMessageBox::warning(
        this,
        tr("Ошибка тарировки"),
        tr("Табличная тарировка должна содержать "
           "не менее двух точек с возрастающими X "
           "и конечными значениями X и Y."));

      return;
    }
  }
  else if (result.kind == CalibrationKind::Polynomial)
  {
    if (result.coefficients.empty())
    {
      QMessageBox::warning(
        this,
        tr("Ошибка тарировки"),
        tr("Полиномиальная тарировка должна содержать "
           "хотя бы один коэффициент."));

      return;
    }
  }

  QDialog::accept();
}

}