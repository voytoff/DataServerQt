#include "basecomboboxdelegate.h"

#include <QComboBox>
#include <QPalette>

namespace qds
{

BaseComboBoxDelegate::BaseComboBoxDelegate(
  std::vector<Item> items,
  QObject* parent)
  : QStyledItemDelegate(parent)
  , m_items(std::move(items))
{
}

QWidget* BaseComboBoxDelegate::createEditor(
  QWidget* parent,
  const QStyleOptionViewItem&,
  const QModelIndex&) const
{
  auto* editor = new QComboBox(parent);

  editor->setAutoFillBackground(true);
  editor->setBackgroundRole(QPalette::Base);

  for (const auto& item : m_items)
  {
    editor->addItem(
      item.text,
      item.value);
  }

  return editor;
}

void BaseComboBoxDelegate::setEditorData(
  QWidget* editor,
  const QModelIndex& index) const
{
  auto* combo = static_cast<QComboBox*>(editor);

  const QVariant value =
    index.data(Qt::EditRole);

  const int current =
    combo->findData(value);

  combo->setCurrentIndex(current);
}

void BaseComboBoxDelegate::setModelData(
  QWidget* editor,
  QAbstractItemModel* model,
  const QModelIndex& index) const
{
  auto* combo = static_cast<QComboBox*>(editor);

  if (combo->currentIndex() < 0)
    return;

  model->setData(
    index,
    combo->currentData(),
    Qt::EditRole);
}

void BaseComboBoxDelegate::setItems(
  std::vector<Item> items)
{
  m_items = std::move(items);
}

}