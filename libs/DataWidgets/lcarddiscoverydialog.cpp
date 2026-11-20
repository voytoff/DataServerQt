#include "lcarddiscoverydialog.h"
#include "ui_lcarddiscoverydialog.h"

#include <QPushButton>
#include <QtConcurrent>
#include <QStatusBar>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QHostAddress>
#include <QHostInfo>
#include <QSignalBlocker>
#include <QAbstractSocket>

#include <utility>

namespace qds
{

LCardDiscoveryDialog::LCardDiscoveryDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::LCardDiscoveryDialog)
{
  ui->setupUi(this);
  this->setSizeGripEnabled(true);

  ui->buttonBox->button(QDialogButtonBox::Reset)
    ->setText(tr("Обновить"));

  QStringList headers = {"Оборудование", "Слот", "Серийный номер", "Состояние"};
  ui->treeWidget->setHeaderLabels(headers);

  //ui->treeWidget->header()->setStretchLastSection(true);
  ui->treeWidget->resizeColumnToContents(0);
  ui->treeWidget->resizeColumnToContents(1);
  ui->treeWidget->resizeColumnToContents(2);
  ui->treeWidget->resizeColumnToContents(3);

  ui->statusLabel->clear();
  ui->progressBar->setRange(0, 0);
  ui->progressBar->hide();

  connect(
    ui->treeWidget,
    &QTreeWidget::itemChanged,
    this,
    &LCardDiscoveryDialog::updateSelection);

  setMode(LCardDiscoveryDialogMode::Browse);

  connect(
    ui->buttonBox->button(QDialogButtonBox::Reset),
    &QPushButton::clicked,
    this,
    &LCardDiscoveryDialog::startScan);

  connect(
    &m_scanWatcher,
    &QFutureWatcher<LCardDiscoveryResult>::finished,
    this,
    &LCardDiscoveryDialog::finishScan);

  connect(
    ui->buttonBox,
    &QDialogButtonBox::accepted,
    this, &QDialog::accept);

  connect(
    ui->buttonBox,
    &QDialogButtonBox::rejected,
    this,
    &QDialog::reject);

  connect(
    ui->treeWidget,
    &QTreeWidget::customContextMenuRequested,
    this,
    &LCardDiscoveryDialog::showContextMenu);

  connect(
    ui->hostEdit,
    &QLineEdit::textChanged,
    this,
    &LCardDiscoveryDialog::invalidateResult);

  connect(
    ui->portSpinBox,
    &QSpinBox::valueChanged,
    this,
    &LCardDiscoveryDialog::invalidateResult);
}

LCardDiscoveryDialog::~LCardDiscoveryDialog()
{
  m_scanWatcher.waitForFinished();
  delete ui;
}

void LCardDiscoveryDialog::setMode(
  LCardDiscoveryDialogMode mode)
{
  m_mode = mode;

  const bool selectable =
    m_mode == LCardDiscoveryDialogMode::Select;

  auto* okButton =
    ui->buttonBox->button(QDialogButtonBox::Ok);

  auto* cancelButton =
    ui->buttonBox->button(QDialogButtonBox::Cancel);

  okButton->setVisible(selectable);
  okButton->setText(tr("Выбрать"));

  cancelButton->setText(
    selectable ? tr("Отмена") : tr("Закрыть"));

  populateTree(m_discoveryResult);

  updateSelection();
}

void LCardDiscoveryDialog::setServer(
  const QString& host,
  uint16_t port)
{
  const QSignalBlocker hostBlocker(ui->hostEdit);
  const QSignalBlocker portBlocker(ui->portSpinBox);

  ui->hostEdit->setText(host);
  ui->portSpinBox->setValue(port);

  invalidateResult();
}

void LCardDiscoveryDialog::setServer(
  uint32_t address,
  uint16_t port)
{
  setServer(QHostAddress(address).toString(), port);
}

QString LCardDiscoveryDialog::host() const
{
  return ui->hostEdit->text().trimmed();
}

uint16_t LCardDiscoveryDialog::port() const
{
  return static_cast<uint16_t>(
    ui->portSpinBox->value());
}

std::vector<LCardDiscoveredCrate>
LCardDiscoveryDialog::selectedCrates() const
{
  std::vector<LCardDiscoveredCrate> result;

  if (m_mode != LCardDiscoveryDialogMode::Select)
    return result;

  for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i)
  {
    const auto* crateItem =
      ui->treeWidget->topLevelItem(i);

    if (crateItem->checkState(0) == Qt::Unchecked)
      continue;

    const auto crateIndex =
      crateItem->data(0, Qt::UserRole).toULongLong();

    const auto& sourceCrate =
      m_discoveryResult.crates[crateIndex];

    auto crate = sourceCrate;
    crate.modules.clear();

    for (int j = 0; j < crateItem->childCount(); ++j)
    {
      const auto* moduleItem = crateItem->child(j);

      if (moduleItem->checkState(0) != Qt::Checked)
        continue;

      const auto moduleIndex =
        moduleItem->data(0, Qt::UserRole).toULongLong();

      crate.modules.push_back(
        sourceCrate.modules[moduleIndex]);
    }

    result.push_back(std::move(crate));
  }

  return result;
}

QString LCardDiscoveryDialog::scanHost() const
{
  return m_scanHost;
}

uint16_t LCardDiscoveryDialog::scanPort() const
{
  return m_scanPort;
}

void LCardDiscoveryDialog::reject()
{
  if (m_scanWatcher.isRunning())
  {
    m_closePending = true;
    ui->statusLabel->setText(
      tr("Ожидание завершения сканирования..."));

    ui->buttonBox->setEnabled(false);
    return;
  }

  QDialog::reject();
}

void LCardDiscoveryDialog::closeEvent(QCloseEvent* event)
{
  if (m_scanWatcher.isRunning())
  {
    m_closePending = true;

    ui->statusLabel->setText(
      tr("Ожидание завершения сканирования..."));

    ui->buttonBox->setEnabled(false);

    event->ignore();
    return;
  }

  QDialog::closeEvent(event);
}

void LCardDiscoveryDialog::populateTree(
  const LCardDiscoveryResult& result)
{
  ui->treeWidget->clear();

  const bool selectable =
    m_mode == LCardDiscoveryDialogMode::Select;

  for (std::size_t i = 0; i < result.crates.size(); ++i)
  {
    const auto& crate = result.crates[i];
    /*
    qDebug()
      << "Crate:"
      << QString::fromStdString(crate.serial)
      << "typeId:" << crate.typeId
      << "interfaceId:" << crate.interfaceId
      << "slotCount:" << crate.slotCount
      << "name:" << QString::fromStdString(crate.name)
      << "typeName:" << QString::fromStdString(crate.typeName);
    */
    auto* crateItem = new QTreeWidgetItem(ui->treeWidget);

    crateItem->setData(
      0, Qt::UserRole, static_cast<qulonglong>(i));

    const auto name = LCardDiscovery::crateName(
      crate.typeId,
      crate.slotCount);

    crateItem->setText(
      0, QString::fromStdString(name));

    crateItem->setText(
      2, QString::fromStdString(crate.serial));

    crateItem->setText(
      3, QString::fromStdString(crate.error));

    for (std::size_t j = 0; j < crate.modules.size(); ++j)
    {
      const auto& module = crate.modules[j];

      auto* moduleItem = new QTreeWidgetItem(crateItem);

      moduleItem->setData(
        0, Qt::UserRole, static_cast<qulonglong>(j));

      moduleItem->setText(
        0,
        QString::fromStdString(
          LCardDiscovery::moduleName(module.typeId)));

      moduleItem->setText(
        1, QString::number(module.slot));

      moduleItem->setText(
        2, QString::fromStdString(module.serial));

      if (selectable)
      {
        moduleItem->setFlags(
          moduleItem->flags() | Qt::ItemIsUserCheckable);

        moduleItem->setCheckState(0, Qt::Checked);
      }
    }

    if (selectable)
    {
      crateItem->setFlags(
        crateItem->flags()
        | Qt::ItemIsUserCheckable
        | Qt::ItemIsAutoTristate);

      crateItem->setCheckState(0, Qt::Checked);
    }

    crateItem->setExpanded(true);
  }

  ui->treeWidget->resizeColumnToContents(0);
  ui->treeWidget->resizeColumnToContents(1);
  ui->treeWidget->resizeColumnToContents(2);
}

void LCardDiscoveryDialog::startScan()
{
  if (m_scanWatcher.isRunning())
    return;

  invalidateResult();

  const auto serverHost = host();
  const auto serverPort = port();

  if (serverHost.isEmpty())
  {
    ui->statusLabel->setText(
      tr("Не указан адрес сервера"));

    ui->hostEdit->setFocus();
    return;
  }

  m_scanHost = serverHost;
  m_scanPort = serverPort;

  ui->statusLabel->setText(
    tr("Сканирование оборудования..."));

  ui->progressBar->show();

  ui->hostEdit->setEnabled(false);
  ui->portSpinBox->setEnabled(false);

  ui->buttonBox->button(QDialogButtonBox::Reset)
    ->setEnabled(false);

  ui->buttonBox->button(QDialogButtonBox::Ok)
    ->setEnabled(false);

  m_scanWatcher.setFuture(
    QtConcurrent::run(
      [serverHost, serverPort]()
      {
        QHostAddress address;

        if (!address.setAddress(serverHost))
        {
          const auto info =
            QHostInfo::fromName(serverHost);

          for (const auto& candidate : info.addresses())
          {
            if (candidate.protocol()
                == QAbstractSocket::IPv4Protocol)
            {
              address = candidate;
              break;
            }
          }
        }

        if (address.protocol()
            != QAbstractSocket::IPv4Protocol)
        {
          LCardDiscoveryResult result;

          result.error =
            tr("Не удалось определить IPv4-адрес сервера: %1")
              .arg(serverHost)
              .toStdString();

          return result;
        }

        return LCardDiscovery::scan(
          address.toIPv4Address(),
          serverPort);
      }));
}

void LCardDiscoveryDialog::finishScan()
{
  ui->hostEdit->setEnabled(true);
  ui->portSpinBox->setEnabled(true);

  if (m_closePending)
  {
    m_closePending = false;
    QDialog::reject();
    return;
  }

  m_discoveryResult = m_scanWatcher.result();
  m_discoveryResult = m_scanWatcher.result();

  populateTree(m_discoveryResult);

  ui->progressBar->hide();

  QString status;

  if (!m_discoveryResult.success())
    status = QString::fromStdString(m_discoveryResult.error);
  else if (m_discoveryResult.crates.empty())
    status = tr("Устройства не обнаружены");
  else
    status = tr("Сканирование завершено");

  ui->statusLabel->setText(status);

  ui->buttonBox->button(QDialogButtonBox::Reset)
    ->setEnabled(true);

  updateSelection();
}

void LCardDiscoveryDialog::updateSelection()
{
  bool enabled = false;

  if (m_mode == LCardDiscoveryDialogMode::Select
      && !m_scanWatcher.isRunning()
      && !m_closePending
      && m_discoveryResult.success())
  {
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i)
    {
      if (ui->treeWidget->topLevelItem(i)->checkState(0)
          != Qt::Unchecked)
      {
        enabled = true;
        break;
      }
    }
  }

  ui->buttonBox->button(QDialogButtonBox::Ok)
    ->setEnabled(enabled);
}

void LCardDiscoveryDialog::invalidateResult()
{
  m_discoveryResult = {};

  m_scanHost.clear();
  m_scanPort = 0;

  ui->treeWidget->clear();
  ui->statusLabel->clear();

  updateSelection();
}

void LCardDiscoveryDialog::showContextMenu(const QPoint& pos)
{
  const auto* item = ui->treeWidget->itemAt(pos);

  if (!item)
    return;

  const QString serial = item->text(2);

  QStringList columns;

  for (int i = 0; i < ui->treeWidget->columnCount(); ++i)
    columns.append(item->text(i));

  QMenu menu(this);

  auto* copySerialAction =
    menu.addAction(tr("Копировать серийный номер"));

  copySerialAction->setEnabled(!serial.isEmpty());

  auto* copyRowAction =
    menu.addAction(tr("Копировать строку"));

  const auto* action =
    menu.exec(ui->treeWidget->viewport()->mapToGlobal(pos));

  if (action == copySerialAction)
  {
    QApplication::clipboard()->setText(serial);
  }
  else if (action == copyRowAction)
  {
    QApplication::clipboard()->setText(columns.join('\t'));
  }
}

}