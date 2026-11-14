#include "paddeditemdelegate.h"

#include <QStyleOptionViewItem>

namespace qds
{

PaddedItemDelegate::PaddedItemDelegate(
  int dx1,
  int dy1,
  int dx2,
  int dy2,
  QObject* parent)
  : QStyledItemDelegate(parent)
  , m_dx1(dx1)
  , m_dy1(dy1)
  , m_dx2(dx2)
  , m_dy2(dy2)
{
}

QSize PaddedItemDelegate::sizeHint(
  const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  QSize size =
    QStyledItemDelegate::sizeHint(option, index);

  size.rwidth() += m_dx1 + m_dx2;
  size.rheight() += m_dy1 + m_dy2;

  return size;
}

void PaddedItemDelegate::initStyleOption(
  QStyleOptionViewItem* option,
  const QModelIndex& index) const
{
  QStyledItemDelegate::initStyleOption(
    option,
    index);

  option->rect.adjust(
    m_dx1, m_dy1, -m_dx2, -m_dy2);
}

}