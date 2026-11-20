#pragma once

#include <QDialog>

namespace Ui {
class CrateDialog;
}

namespace qds
{

class CrateDialog : public QDialog
{
  Q_OBJECT

public:
  explicit CrateDialog(QWidget *parent = nullptr);
  ~CrateDialog();

private:
  Ui::CrateDialog *ui;
};

}
