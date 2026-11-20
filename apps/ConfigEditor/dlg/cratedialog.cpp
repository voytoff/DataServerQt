#include "cratedialog.h"
#include "ui_cratedialog.h"

namespace qds
{

CrateDialog::CrateDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::CrateDialog)
{
  ui->setupUi(this);
}

CrateDialog::~CrateDialog()
{
  delete ui;
}

}