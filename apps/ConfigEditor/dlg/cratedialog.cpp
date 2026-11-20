#include "cratedialog.h"
#include "ui_cratedialog.h"
#include "cratetypeinfo.h"

#include <QPushButton>

namespace qds
{

CrateDialog::CrateDialog(
  QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::CrateDialog)
{
  ui->setupUi(this);
  this->setSizeGripEnabled(true);

  setWindowTitle(tr("Добавление крейта"));

  constexpr CrateType types[] =
    {
      CrateType::LTR_U_8,
      CrateType::LTR_U_16,
      CrateType::LTR_U_1,
      CrateType::LTR_EU_8,
      CrateType::LTR_EU_16,
      CrateType::LTR_EU_2,
      CrateType::LTR_E_7,
      CrateType::LTR_E_15,
      CrateType::LTR_CU_1,
      CrateType::LTR_CEU_1
    };

  for (const auto type : types)
  {
    ui->crateTypeComboBox->addItem(
      crateTypeName(type),
      static_cast<int>(type));
  }

  ui->crateTypeComboBox->setCurrentIndex(-1);

  connect(
    ui->crateTypeComboBox,
    &QComboBox::currentIndexChanged,
    this,
    &CrateDialog::updateActions);

  connect(
    ui->serialLineEdit,
    &QLineEdit::textChanged,
    this,
    &CrateDialog::updateActions);

  connect(
    ui->hostLineEdit,
    &QLineEdit::textChanged,
    this,
    &CrateDialog::updateActions);

  connect(
    ui->buttonBox,
    &QDialogButtonBox::accepted,
    this,
    &QDialog::accept);

  updateActions();
}

CrateDialog::~CrateDialog()
{
  delete ui;
}

CrateType CrateDialog::crateType() const
{
  if (ui->crateTypeComboBox->currentIndex() < 0)
    return CrateType::Unknown;

  return static_cast<CrateType>(
    ui->crateTypeComboBox->currentData().toInt());
}

QString CrateDialog::serial() const
{
  return ui->serialLineEdit->text().trimmed();
}

QString CrateDialog::host() const
{
  return ui->hostLineEdit->text().trimmed();
}

uint32_t CrateDialog::port() const
{
  return static_cast<uint32_t>(
    ui->portSpinBox->value());
}

QString CrateDialog::description() const
{
  return ui->descriptionLineEdit->text();
}

void CrateDialog::updateActions()
{
  const bool valid =
    crateType() != CrateType::Unknown &&
    !serial().isEmpty() &&
    !host().isEmpty();

  ui->buttonBox->button(
                 QDialogButtonBox::Ok)->setEnabled(valid);
}

}