#pragma once

#include <QStyledItemDelegate>

namespace qds
{

class ConfigurationPropertiesDelegate final
  : public QStyledItemDelegate
{
public:
  explicit ConfigurationPropertiesDelegate(
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

  void updateEditorGeometry(
    QWidget* editor,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;
};

}