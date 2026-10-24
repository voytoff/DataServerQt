#include "basetableview.h"

#include <QDebug>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QStringList>
#include <QVector>
#include <QFrame>

#include <algorithm>
#include <utility>

namespace qds
{

BaseTableView::BaseTableView(
  int columnWidth,
  QWidget *parent)
  : QTableView(parent)
{
  setFrameShape(
    QFrame::NoFrame);

  setSelectionBehavior(
    QAbstractItemView::SelectRows);

  setSelectionMode(
    QAbstractItemView::SingleSelection);

  setStyleSheet(
    "QTableView::item {padding: 0 4px 0 4px;}");

  auto *vHeader = verticalHeader();

  vHeader->setSectionResizeMode(
    QHeaderView::Fixed);

  vHeader->setDefaultSectionSize(26);

  vHeader->setVisible(false);

  auto *hHeader = horizontalHeader();

  hHeader->setSectionResizeMode(
    QHeaderView::Interactive);

  if (columnWidth > 0)
    hHeader->setDefaultSectionSize(columnWidth);
}

BaseTableView::BaseTableView(QWidget* parent)
  : BaseTableView(0, parent)
{
}

void BaseTableView::setPasteEnabled(
  bool enabled) noexcept
{
  m_pasteEnabled = enabled;
}

bool BaseTableView::pasteEnabled() const noexcept
{
  return m_pasteEnabled;
}

bool BaseTableView::pasteFromClipboard()
{
  if (!m_pasteEnabled || !model())
    return false;

  const QModelIndex current = currentIndex();

  if (!current.isValid())
    return false;

  QString text =
    QApplication::clipboard()->text();

  if (text.isEmpty())
    return false;

  text.replace("\r\n", "\n");
  text.replace('\r', '\n');

  if (text.endsWith('\n'))
    text.chop(1);

  if (text.isEmpty())
    return false;

  const QStringList rows =
    text.split('\n', Qt::KeepEmptyParts);

  const int firstRow = current.row();
  const int firstColumn = current.column();

  QVector<QStringList> cells;
  cells.reserve(rows.size());

  int columnCount = -1;

  for (const QString& row : rows)
  {
    QStringList values =
      row.split('\t', Qt::KeepEmptyParts);

    if (columnCount < 0)
      columnCount = values.size();
    else if (values.size() != columnCount)
      return false;

    cells.push_back(std::move(values));
  }

  if (firstColumn + columnCount > model()->columnCount())
    return false;

  const int existingRows = model()->rowCount();

  const int requiredRows =
    firstRow + static_cast<int>(cells.size());

  const int missingRows =
    std::max(0, requiredRows - existingRows);

  if (missingRows > 0 && !m_pasteInsertRowsEnabled)
    return false;

  // Проверяем редактируемость существующих ячеек.
  for (int row = 0; row < cells.size(); ++row)
  {
    const int targetRow = firstRow + row;

    if (targetRow >= existingRows)
      break;

    for (int column = 0;
         column < cells[row].size();
         ++column)
    {
      const QModelIndex index =
        model()->index(
          targetRow,
          firstColumn + column);

      if (!(model()->flags(index) & Qt::ItemIsEditable))
        return false;
    }
  }

  // Добавляем недостающие строки в конец.
  if (missingRows > 0)
  {
    if (!model()->insertRows(
          existingRows,
          missingRows))
    {
      return false;
    }
  }

  // Проверяем редактируемость новых ячеек.
  for (int row = existingRows;
       row < requiredRows;
       ++row)
  {
    for (int column = 0;
         column < columnCount;
         ++column)
    {
      const QModelIndex index =
        model()->index(
          row,
          firstColumn + column);

      if (!(model()->flags(index) & Qt::ItemIsEditable))
        return false;
    }
  }

  // Записываем значения.
  for (int row = 0; row < cells.size(); ++row)
  {
    for (int column = 0;
         column < cells[row].size();
         ++column)
    {
      const QModelIndex index =
        model()->index(
          firstRow + row,
          firstColumn + column);

      if (!model()->setData(
            index,
            cells[row][column],
            Qt::EditRole))
      {
        return false;
      }
    }
  }

  return true;
}

void BaseTableView::setPasteInsertRowsEnabled(
  bool enabled) noexcept
{
  m_pasteInsertRowsEnabled = enabled;
}

void BaseTableView::keyPressEvent(
  QKeyEvent* event)
{
  if (m_pasteEnabled &&
      event->matches(QKeySequence::Paste))
  {
    pasteFromClipboard();
    event->accept();
    return;
  }

  QTableView::keyPressEvent(event);
}

}