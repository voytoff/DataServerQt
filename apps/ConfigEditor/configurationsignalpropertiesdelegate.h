#pragma once

#include "datatypes.h"
#include <QStyledItemDelegate>

namespace qds
{

class ConfigurationSignalPropertiesDelegate final
  : public QStyledItemDelegate
{
public:
  explicit ConfigurationSignalPropertiesDelegate(
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

  void setTagSources(
    std::vector<TagSourceInfo> sources);

  void setSignalTypes(
    std::vector<SignalTypeInfo> types);

private:
  std::vector<SignalTypeInfo> m_signalTypes;
  std::vector<TagSourceInfo> m_tagSources;

};

}
