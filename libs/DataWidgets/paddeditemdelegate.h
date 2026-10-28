#pragma once

#include "datawidgets_export.h"

#include <QStyledItemDelegate>

namespace qds
{

class DATAWIDGETS_EXPORT PaddedItemDelegate
  : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit PaddedItemDelegate(
    int dx1 = 4,
    int dy1 = 0,
    int dx2 = 4,
    int dy2 = 0,
    QObject* parent = nullptr);

protected:
  void initStyleOption(
    QStyleOptionViewItem* option,
    const QModelIndex& index) const override;

private:
  int m_dx1;
  int m_dy1;
  int m_dx2;
  int m_dy2;
};

}