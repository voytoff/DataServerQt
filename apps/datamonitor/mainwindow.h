#pragma once

#include <QMainWindow>

namespace qds
{

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

signals:
};

}
