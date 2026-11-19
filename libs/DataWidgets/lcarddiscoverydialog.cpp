#include "lcarddiscoverydialog.h"
#include "ui_lcarddiscoverydialog.h"

#include <QPushButton>
#include <QtConcurrent>

namespace qds
{

LCardDiscoveryDialog::LCardDiscoveryDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::LCardDiscoveryDialog)
{
  ui->setupUi(this);
  this->setWindowTitle("Обнаружение оборудования LCard");

  ui->buttonBox->button(QDialogButtonBox::Reset)
    ->setText(tr("Обновить"));

  QStringList headers = {"Оборудование", "Слот", "Серийный номер", "Состояние"};
  ui->treeWidget->setHeaderLabels(headers);

  ui->statusLabel->clear();
  ui->progressBar->setRange(0, 0);
  ui->progressBar->hide();

  setMode(LCardDiscoveryDialogMode::Browse);

  connect(
    ui->buttonBox->button(QDialogButtonBox::Reset),
    &QPushButton::clicked,
    this,
    &LCardDiscoveryDialog::startScan);

  connect(
    &scanWatcher_,
    &QFutureWatcher<LCardDiscoveryResult>::finished,
    this,
    &LCardDiscoveryDialog::finishScan);

  connect(ui->buttonBox, &QDialogButtonBox::accepted,
          this, &QDialog::accept);

  connect(ui->buttonBox, &QDialogButtonBox::rejected,
          this, &QDialog::reject);
}

LCardDiscoveryDialog::~LCardDiscoveryDialog()
{
  scanWatcher_.waitForFinished();
  delete ui;
}

void LCardDiscoveryDialog::setMode(LCardDiscoveryDialogMode mode)
{
  mode_ = mode;

  ui->buttonBox->button(QDialogButtonBox::Ok)
    ->setVisible(mode == LCardDiscoveryDialogMode::Select);
}

void LCardDiscoveryDialog::setServer(
  uint32_t address,
  uint16_t port)
{
  address_ = address;
  port_ = port;
}

void LCardDiscoveryDialog::populateTree(
  const LCardDiscoveryResult& result)
{
  ui->treeWidget->clear();

  for (const auto& crate : result.crates)
  {
    auto* crateItem = new QTreeWidgetItem(ui->treeWidget);

    crateItem->setText(
      0,
      tr("Крейт %1").arg(QString::fromStdString(crate.serial)));

    crateItem->setText(
      2,
      QString::fromStdString(crate.serial));

    crateItem->setText(
      3,
      QString::fromStdString(crate.error));

    if (mode_ == LCardDiscoveryDialogMode::Select)
    {
      crateItem->setFlags(
        crateItem->flags() | Qt::ItemIsUserCheckable);

      crateItem->setCheckState(0, Qt::Checked);
    }

    for (const auto& module : crate.modules)
    {
      auto* moduleItem = new QTreeWidgetItem(crateItem);

      moduleItem->setText(
        0,
        QString::fromStdString(LCardDiscovery::moduleName(module.typeId)));

      moduleItem->setText(
        1,
        QString::number(module.slot));

      moduleItem->setText(
        2,
        QString::fromStdString(module.serial));

      if (mode_ == LCardDiscoveryDialogMode::Select)
      {
        moduleItem->setFlags(
          moduleItem->flags() | Qt::ItemIsUserCheckable);

        moduleItem->setCheckState(0, Qt::Checked);
      }
    }

    crateItem->setExpanded(true);
  }

  ui->treeWidget->resizeColumnToContents(0);
  ui->treeWidget->resizeColumnToContents(1);
  ui->treeWidget->resizeColumnToContents(2);
}

void LCardDiscoveryDialog::startScan()
{
  if (scanWatcher_.isRunning())
    return;

  ui->treeWidget->clear();
  ui->statusLabel->setText(tr("Сканирование оборудования..."));
  ui->progressBar->show();

  ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

  const auto address = address_;
  const auto port = port_;

  scanWatcher_.setFuture(
    QtConcurrent::run([address, port]()
                      {
                        return LCardDiscovery::scan(address, port);
                      }));
}

void LCardDiscoveryDialog::finishScan()
{
  discoveryResult_ = scanWatcher_.result();

  populateTree(discoveryResult_);

  ui->progressBar->hide();

  ui->statusLabel->setText(
    discoveryResult_.success()
      ? tr("Сканирование завершено")
      : QString::fromStdString(discoveryResult_.error));

  ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(true);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

}