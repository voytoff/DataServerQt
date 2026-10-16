#include "calibrationdialog.h"
#include "basetableview.h"
#include "plot/calibrationplot.h"

#include <QSplitter>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QTimer>

#include <algorithm>
#include <functional>
#include <vector>

namespace qds
{

CalibrationDialog::CalibrationDialog(
  const Calibration& calibration,
  QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle(tr("Редактор тарировки"));
  resize(1000, 600);

  m_model = new CalibrationTableModel(this);
  m_model->setCalibration(calibration);

  m_nameEdit = new QLineEdit(this);
  m_nameEdit->setText(calibration.name);

  m_descriptionEdit = new QTextEdit(this);
  m_descriptionEdit->setPlainText(
    calibration.description);
  m_descriptionEdit->setMaximumHeight(65);

  m_view = new BaseTableView(150, this);
  m_view->setModel(m_model);
  m_view->setPasteEnabled(true);
  m_view->setPasteInsertRowsEnabled(true);

  m_view->setSelectionBehavior(
    QAbstractItemView::SelectRows);

  m_view->setSelectionMode(
    QAbstractItemView::ExtendedSelection);

  m_view->horizontalHeader()->setStretchLastSection(true);

  m_addButton = new QPushButton(
    tr("Добавить строку"), this);

  m_removeButton = new QPushButton(
    tr("Удалить строки"), this);

  auto* rowButtons = new QHBoxLayout;
  rowButtons->addWidget(m_addButton);
  rowButtons->addWidget(m_removeButton);
  rowButtons->addStretch();


  // Левая часть: таблица и кнопки.
  auto* tableWidget = new QWidget(this);

  auto* tableLayout = new QVBoxLayout(tableWidget);
  tableLayout->setContentsMargins(0, 0, 0, 0);

  tableLayout->addWidget(m_view);
  tableLayout->addLayout(rowButtons);

  // Правая часть: график.
  m_plot = new CalibrationPlot(this);

  // Разделитель таблицы и графика.
  auto* splitter = new QSplitter(
    Qt::Horizontal, this);

  splitter->addWidget(tableWidget);
  splitter->addWidget(m_plot);

  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 2);

  splitter->setSizes({350, 650});


  m_buttons = new QDialogButtonBox(
    QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel,
    this);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(10, 10, 10, 10);

  layout->addWidget(new QLabel(tr("Название:"), this));
  layout->addWidget(m_nameEdit);

  layout->addWidget(new QLabel(tr("Описание:"), this));
  layout->addWidget(m_descriptionEdit);

  layout->addWidget(splitter, 1);
  layout->addWidget(m_buttons);

  m_plotTimer = new QTimer(this);
  m_plotTimer->setSingleShot(true);
  m_plotTimer->setInterval(0);

  connect(
    m_addButton,
    &QPushButton::clicked,
    this,
    &CalibrationDialog::addRow);

  connect(
    m_removeButton,
    &QPushButton::clicked,
    this,
    &CalibrationDialog::removeRows);

  connect(
    m_buttons,
    &QDialogButtonBox::accepted,
    this,
    &CalibrationDialog::accept);

  connect(
    m_buttons,
    &QDialogButtonBox::rejected,
    this,
    &QDialog::reject);


  connect(
    m_model,
    &QAbstractItemModel::dataChanged,
    m_plotTimer,
    qOverload<>(&QTimer::start));

  connect(
    m_model,
    &QAbstractItemModel::rowsInserted,
    m_plotTimer,
    qOverload<>(&QTimer::start));

  connect(
    m_model,
    &QAbstractItemModel::rowsRemoved,
    m_plotTimer,
    qOverload<>(&QTimer::start));

  connect(
    m_model,
    &QAbstractItemModel::modelReset,
    m_plotTimer,
    qOverload<>(&QTimer::start));

  connect(
    m_plotTimer,
    &QTimer::timeout,
    this,
    &CalibrationDialog::updatePlot);

  // Первоначальное построение графика.
  updatePlot();
}

Calibration CalibrationDialog::calibration() const
{
  Calibration result = m_model->calibration();

  result.name = m_nameEdit->text().trimmed();
  result.description =
    m_descriptionEdit->toPlainText();

  return result;
}

void CalibrationDialog::addRow()
{
  const QModelIndex current =
    m_view->currentIndex();

  const int row =
    current.isValid()
      ? current.row() + 1
      : m_model->rowCount();

  if (!m_model->insertRow(row))
    return;

  m_view->selectRow(row);

  const int column =
    m_model->calibration().kind ==
        CalibrationKind::Polynomial
      ? 1
      : 0;

  const QModelIndex index =
    m_model->index(row, column);

  m_view->setCurrentIndex(index);
  m_view->edit(index);
}

void CalibrationDialog::removeRows()
{
  const auto selected =
    m_view->selectionModel()->selectedRows();

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
  m_plot->setCalibration(
    m_model->calibration());
}

void CalibrationDialog::accept()
{
  Calibration result = calibration();

  if (result.name.isEmpty())
  {
    QMessageBox::warning(
      this,
      tr("Ошибка тарировки"),
      tr("Название тарировки не может быть пустым."));

    m_nameEdit->setFocus();
    return;
  }

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