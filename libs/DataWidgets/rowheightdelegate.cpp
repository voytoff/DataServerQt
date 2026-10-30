#include "rowheightdelegate.h"

namespace qds
{

RowHeightDelegate::RowHeightDelegate(
  int rowHeight,
  QObject *parent)
: QStyledItemDelegate(parent)
  , m_rowHeight(rowHeight)
{
}

QSize RowHeightDelegate::sizeHint(
  const QStyleOptionViewItem &option,
  const QModelIndex &index) const
{
  QSize size = QStyledItemDelegate::sizeHint(option, index);
  size.setHeight(m_rowHeight);
  return size;
}

}