#include "calibrationtablemodel.h"

#include <cmath>
#include <cstdint>
#include <QLocale>

namespace qds
{

CalibrationTableModel::CalibrationTableModel(
  QObject* parent)
  : QAbstractTableModel(parent)
{
}

void CalibrationTableModel::setCalibration(
  const Calibration& calibration)
{
  beginResetModel();

  m_calibration = calibration;

  if (m_calibration.kind == CalibrationKind::Table)
    updatePointIndices();

  endResetModel();
}

const Calibration& CalibrationTableModel::calibration()
  const noexcept
{
  return m_calibration;
}

int CalibrationTableModel::rowCount(
  const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  switch (m_calibration.kind)
  {
  case CalibrationKind::Table:
    return static_cast<int>(
      m_calibration.points.size());

  case CalibrationKind::Polynomial:
    return static_cast<int>(
      m_calibration.coefficients.size());
  }

  return 0;
}

int CalibrationTableModel::columnCount(
  const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 2;
}

QVariant CalibrationTableModel::data(
  const QModelIndex& index,
  int role) const
{
  if (!index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= rowCount() ||
      index.column() < 0 ||
      index.column() >= columnCount())
  {
    return {};
  }

  if (role != Qt::DisplayRole &&
      role != Qt::EditRole)
  {
    return {};
  }

  const auto row =
    static_cast<std::size_t>(index.row());

  if (m_calibration.kind == CalibrationKind::Table)
  {
    const auto& point =
      m_calibration.points[row];

    return index.column() == 0
             ? point.x
             : point.y;
  }

  if (m_calibration.kind == CalibrationKind::Polynomial)
  {
    if (index.column() == 0)
    {
      if (role == Qt::DisplayRole)
        return QString("a%1").arg(index.row());

      return index.row();
    }

    return m_calibration.coefficients[row];
  }

  return {};
}

QVariant CalibrationTableModel::headerData(
  int section,
  Qt::Orientation orientation,
  int role) const
{
  if (role != Qt::DisplayRole)
    return {};

  if (orientation == Qt::Vertical)
    return section + 1;

  if (orientation != Qt::Horizontal)
    return {};

  if (m_calibration.kind == CalibrationKind::Table)
  {
    switch (section)
    {
    case 0: return tr("X");
    case 1: return tr("Y");
    }
  }
  else if (
    m_calibration.kind == CalibrationKind::Polynomial)
  {
    switch (section)
    {
    case 0: return tr("Коэффициент");
    case 1: return tr("Значение");
    }
  }

  return {};
}

Qt::ItemFlags CalibrationTableModel::flags(
  const QModelIndex& index) const
{
  auto result =
    QAbstractTableModel::flags(index);

  if (!index.isValid())
    return result;

  if (m_calibration.kind == CalibrationKind::Table ||
      (m_calibration.kind == CalibrationKind::Polynomial &&
       index.column() == 1))
  {
    result |= Qt::ItemIsEditable;
  }

  return result;
}

bool CalibrationTableModel::setData(
  const QModelIndex& index,
  const QVariant& value,
  int role)
{
  if (role != Qt::EditRole ||
      !index.isValid() ||
      index.model() != this ||
      index.row() < 0 ||
      index.row() >= rowCount() ||
      index.column() < 0 ||
      index.column() >= columnCount())
  {
    return false;
  }

  if (!(flags(index) & Qt::ItemIsEditable))
    return false;

  bool ok = false;
  double number = 0.0;

  if (value.metaType().id() == QMetaType::QString)
  {
    QString text = value.toString().trimmed();

    // Поддержка десятичной запятой.
    text.replace(',', '.');

    number = QLocale::c().toDouble(text, &ok);
  }
  else
  {
    number = value.toDouble(&ok);
  }

  if (!ok || !std::isfinite(number))
    return false;

  const auto row =
    static_cast<std::size_t>(index.row());

  if (m_calibration.kind == CalibrationKind::Table)
  {
    auto& point =
      m_calibration.points[row];

    double& target =
      index.column() == 0
        ? point.x
        : point.y;

    if (target == number)
      return true;

    target = number;
  }
  else if (
    m_calibration.kind == CalibrationKind::Polynomial)
  {
    double& target =
      m_calibration.coefficients[row];

    if (target == number)
      return true;

    target = number;
  }
  else
  {
    return false;
  }

  // Ранее рассчитанные сегменты больше не актуальны.
  m_calibration.segments.clear();

  emit dataChanged(
    index,
    index,
    {Qt::DisplayRole, Qt::EditRole});

  return true;
}

bool CalibrationTableModel::insertRows(
  int row,
  int count,
  const QModelIndex& parent)
{
  if (m_calibration.kind != CalibrationKind::Table &&
      m_calibration.kind != CalibrationKind::Polynomial)
    return false;

  if (parent.isValid() ||
      count <= 0 ||
      row < 0 ||
      row > rowCount())
  {
    return false;
  }

  beginInsertRows(
    {},
    row,
    row + count - 1);

  if (m_calibration.kind == CalibrationKind::Table)
  {
    m_calibration.points.insert(
      m_calibration.points.begin() + row,
      count,
      CalibrationPoint{0, 0.0, 0.0});

    updatePointIndices();
    m_calibration.segments.clear();
  }
  else if (
    m_calibration.kind == CalibrationKind::Polynomial)
  {
    m_calibration.coefficients.insert(
      m_calibration.coefficients.begin() + row,
      count,
      0.0);
  }

  endInsertRows();

  return true;
}

bool CalibrationTableModel::removeRows(
  int row,
  int count,
  const QModelIndex& parent)
{
  if (m_calibration.kind != CalibrationKind::Table &&
      m_calibration.kind != CalibrationKind::Polynomial)
    return false;

  if (parent.isValid() ||
      count <= 0 ||
      row < 0 ||
      count > rowCount() - row)
  {
    return false;
  }

  beginRemoveRows(
    {},
    row,
    row + count - 1);

  if (m_calibration.kind == CalibrationKind::Table)
  {
    m_calibration.points.erase(
      m_calibration.points.begin() + row,
      m_calibration.points.begin() + row + count);

    updatePointIndices();
    m_calibration.segments.clear();
  }
  else if (
    m_calibration.kind == CalibrationKind::Polynomial)
  {
    m_calibration.coefficients.erase(
      m_calibration.coefficients.begin() + row,
      m_calibration.coefficients.begin() + row + count);
  }

  endRemoveRows();

  return true;
}

void CalibrationTableModel::updatePointIndices()
{
  for (std::size_t i = 0;
       i < m_calibration.points.size();
       ++i)
  {
    m_calibration.points[i].index =
      static_cast<uint32_t>(i);
  }
}

}