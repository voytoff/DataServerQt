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
}

void BaseTableView::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Delete) {
    qDebug() << "Нажата клавиша Delete. Здесь можно удалить строку из модели.";
    // Вы можете обработать событие здесь или передать наверх, если нужно:
    // event->accept();
    // return;
  }

  // Обязательно вызываем базовый класс, чтобы не сломать стандартную навигацию клавишами
  QTableView::keyPressEvent(event);
}

}