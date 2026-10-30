#pragma once

#include "datawidgets_export.h"

#include <QStyledItemDelegate>

namespace qds
{

class DATAWIDGETS_EXPORT RowHeightDelegate
  : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit RowHeightDelegate(
    int rowHeight = 26,
    QObject* parent = nullptr);

  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
  int m_rowHeight;
};

}