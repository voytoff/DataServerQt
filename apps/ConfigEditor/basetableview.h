#pragma once

#include <QTableView>
#include <QKeyEvent>
#include <QMouseEvent>

namespace qds
{

class BaseTableView : public QTableView
{
  Q_OBJECT

public:
  explicit BaseTableView(
    int columnWidth = 0,
    QWidget *parent = nullptr);

  virtual ~BaseTableView() = default;

protected:
  void keyPressEvent(QKeyEvent *event) override;

};

}
