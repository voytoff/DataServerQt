
#include "archivetableexportdialog.h"
#include "paddeditemdelegate.h"
#include "ui_archivetableexportdialog.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidgetItem>

#include <algorithm>
#include <utility>

namespace qds
{

ArchiveTableExportDialog::ArchiveTableExportDialog(
  const ArchiveTableExportSettings& settings,
  QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::ArchiveTableExportDialog)
  , m_settings(settings)
{
  ui->setupUi(this);

  ui->xlsxRadioButton->setChecked(
    settings.format == ArchiveTableExportFormat::Xlsx);

  ui->odsRadioButton->setChecked(
    settings.format == ArchiveTableExportFormat::Ods);

  for (const uint32_t frequency :
       {1u, 10u, 100u, 1000u})
  {
    ui->frequencyComboBox->addItem(
      tr("%1 Гц").arg(frequency),
      frequency);
  }

  const int frequencyIndex =
    ui->frequencyComboBox->findData(settings.frequency);

  if (frequencyIndex >= 0)
    ui->frequencyComboBox->setCurrentIndex(frequencyIndex);

  ui->showUnitsCheckBox->setChecked(
    settings.showUnitsInHeaders);

  auto* table = ui->channelsTableWidget;

  table->setItemDelegate(
    new PaddedItemDelegate(4, 0, 4, 0, this));


  table->setColumnCount(2);
  table->setHorizontalHeaderLabels({
    QString(),
    tr("Канал")
  });

  table->verticalHeader()->setVisible(false);
  table->verticalHeader()->setDefaultSectionSize(22);

  table->horizontalHeader()->setVisible(false);

  table->setSelectionBehavior(
    QAbstractItemView::SelectRows);

  table->setSelectionMode(
    QAbstractItemView::SingleSelection);

  table->setEditTriggers(
    QAbstractItemView::NoEditTriggers);

  table->horizontalHeader()->setSectionResizeMode(
    0,
    QHeaderView::Fixed);

  table->setColumnWidth(0, 30);

  table->horizontalHeader()->setSectionResizeMode(
    1,
    QHeaderView::Stretch);

  table->setRowCount(
    static_cast<int>(settings.channels.size()));

  for (int row = 0;
       row < table->rowCount();
       ++row)
  {
    const auto& channel = settings.channels[row];

    auto* checkItem = new QTableWidgetItem();

    checkItem->setFlags(
      Qt::ItemIsEnabled |
      Qt::ItemIsUserCheckable |
      Qt::ItemIsSelectable);

    checkItem->setCheckState(Qt::Checked);

    table->setItem(row, 0, checkItem);

    auto* nameItem = new QTableWidgetItem(
      channel.name);

    nameItem->setData(
      Qt::UserRole,
      static_cast<uint32_t>(row));

    table->setItem(row, 1, nameItem);
  }

  if (table->rowCount() > 0)
    table->selectRow(0);

  ui->buttonBox->button(
                 QDialogButtonBox::Ok)->setText(
      tr("Экспортировать"));

  connect(
    ui->buttonBox,
    &QDialogButtonBox::accepted,
    this,
    &QDialog::accept);

  connect(
    ui->buttonBox,
    &QDialogButtonBox::rejected,
    this,
    &QDialog::reject);

  connect(
    ui->moveUpButton,
    &QToolButton::clicked,
    this,
    [this]()
    {
      moveChannel(-1);
    });

  connect(
    ui->moveDownButton,
    &QToolButton::clicked,
    this,
    [this]()
    {
      moveChannel(1);
    });
}

ArchiveTableExportDialog::~ArchiveTableExportDialog()
{
  delete ui;
}

void ArchiveTableExportDialog::moveChannel(int offset)
{
  auto* table = ui->channelsTableWidget;

  const int row = table->currentRow();
  const int target = row + offset;

  if (row < 0 ||
      target < 0 ||
      target >= table->rowCount())
  {
    return;
  }

  for (int column = 0;
       column < table->columnCount();
       ++column)
  {
    QTableWidgetItem* first =
      table->takeItem(row, column);

    QTableWidgetItem* second =
      table->takeItem(target, column);

    table->setItem(row, column, second);
    table->setItem(target, column, first);
  }

  table->selectRow(target);
}

ArchiveTableExportSettings
ArchiveTableExportDialog::settings() const
{
  ArchiveTableExportSettings result = m_settings;

  result.format =
    ui->xlsxRadioButton->isChecked()
      ? ArchiveTableExportFormat::Xlsx
      : ArchiveTableExportFormat::Ods;

  result.frequency =
    ui->frequencyComboBox->currentData().toUInt();

  result.showUnitsInHeaders =
    ui->showUnitsCheckBox->isChecked();

  result.channels.clear();

  const auto* table = ui->channelsTableWidget;

  for (int row = 0;
       row < table->rowCount();
       ++row)
  {
    const auto* checkItem = table->item(row, 0);
    const auto* nameItem = table->item(row, 1);

    if (!checkItem || !nameItem ||
        checkItem->checkState() != Qt::Checked)
    {
      continue;
    }

    const auto index =
      nameItem->data(Qt::UserRole).toUInt();

    if (index >= m_settings.channels.size())
      continue;

    result.channels.push_back(
      m_settings.channels[index]);
  }

  return result;
}

}
