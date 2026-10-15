#pragma once

#include <QStyledItemDelegate>
#include <QVariant>
#include <QString>

#include <vector>

namespace qds
{

class BaseComboBoxDelegate : public QStyledItemDelegate
{
public:
  struct Item
  {
    QString text;
    QVariant value;
  };

  explicit BaseComboBoxDelegate(
    std::vector<Item> items,
    QObject* parent = nullptr);

  QWidget* createEditor(
    QWidget* parent,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;

  void setEditorData(
    QWidget* editor,
    const QModelIndex& index) const override;

  void setModelData(
    QWidget* editor,
    QAbstractItemModel* model,
    const QModelIndex& index) const override;

  void setItems(
    std::vector<Item> items);

private:
  std::vector<Item> m_items;
};

}