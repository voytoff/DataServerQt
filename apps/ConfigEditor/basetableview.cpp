#include "basetableview.h"
#include <QDebug>
#include <QHeaderView>

namespace qds
{

BaseTableView::BaseTableView(
  int columnWidth,
  QWidget *parent)
  : QTableView(parent)
{
  auto *vHeader = verticalHeader();

  vHeader->setSectionResizeMode(
    QHeaderView::Fixed);

  vHeader->setDefaultSectionSize(26);

  vHeader->setVisible(false);

  setSelectionBehavior(
    QAbstractItemView::SelectRows);

  setSelectionMode(
    QAbstractItemView::SingleSelection);

  //setAlternatingRowColors(true);

  auto *hHeader = horizontalHeader();

  hHeader->setSectionResizeMode(
    QHeaderView::Interactive);
  //  QHeaderView::Stretch);

  if (columnWidth > 0)
    hHeader->setDefaultSectionSize(columnWidth);

  // продолжает последнюю колонку до границы
  //hHeader->setStretchLastSection(true);

  //hHeader->setSectionResizeMode(
  //  QHeaderView::ResizeToContents);
}

void BaseTableView::keyPressEvent(QKeyEvent *event)
{
  QTableView::keyPressEvent(event);
}

}