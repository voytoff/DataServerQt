#include "configurationpropertiesdelegate.h"
#include "configurationpropertiesmodel.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QDoubleSpinBox>

namespace qds
{

using Model = ConfigurationPropertiesModel;

ConfigurationPropertiesDelegate::ConfigurationPropertiesDelegate(
  QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* ConfigurationPropertiesDelegate::createEditor(
  QWidget* parent,
  const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  if (!index.isValid() || index.column() != 1)
    return nullptr;

  const auto propertyId =
    static_cast<Model::PropertyId>(
      index.data(Model::PropertyIdRole).toInt());

  if (propertyId == Model::PropertyId::ChannelRate)
  {
    auto* editor = new QDoubleSpinBox(parent);

    editor->setDecimals(2);
    editor->setRange(0.01, 400000.0);
    editor->setSingleStep(100.0);
    editor->setSuffix(tr(" Гц"));

    return editor;
  }
  else if (propertyId == Model::PropertyId::ModuleMode)
  {
    auto* editor = new QComboBox(parent);

    editor->setAutoFillBackground(true);
    editor->setBackgroundRole(QPalette::Base);

    editor->addItem(tr("Дифференциальный"), 0);
    editor->addItem(tr("Общая земля"), 1);

    return editor;
  }
  else if (propertyId == Model::PropertyId::ChannelRange)
  {
    auto* editor = new QComboBox(parent);

    editor->setAutoFillBackground(true);
    editor->setBackgroundRole(QPalette::Base);

    editor->addItem(tr("±10 В"), 0);
    editor->addItem(tr("±2,5 В"), 1);
    editor->addItem(tr("±0,625 В"), 2);
    editor->addItem(tr("±0,156 В"), 3);

    return editor;
  }

  return QStyledItemDelegate::createEditor(
    parent,
    option,
    index);
}

void ConfigurationPropertiesDelegate::setEditorData(
  QWidget* editor,
  const QModelIndex& index) const
{
  const QVariant value =
    index.data(Qt::EditRole);

  if (auto* spin =
      qobject_cast<QDoubleSpinBox*>(editor))
  {
    spin->setValue(value.toDouble());
    return;
  }
  else if (auto* combo =
      qobject_cast<QComboBox*>(editor))
  {
    const int position =
      combo->findData(value.toInt());

    if (position >= 0)
      combo->setCurrentIndex(position);

    return;
  }

  QStyledItemDelegate::setEditorData(
    editor,
    index);
}

void ConfigurationPropertiesDelegate::setModelData(
  QWidget* editor,
  QAbstractItemModel* model,
  const QModelIndex& index) const
{
  if (auto* spin =
      qobject_cast<QDoubleSpinBox*>(editor))
  {
    spin->interpretText();

    model->setData(
      index,
      spin->value(),
      Qt::EditRole);

    return;
  }
  else if (auto* combo =
      qobject_cast<QComboBox*>(editor))
  {
    model->setData(
      index,
      combo->currentData(),
      Qt::EditRole);

    return;
  }

  QStyledItemDelegate::setModelData(
    editor,
    model,
    index);
}

void ConfigurationPropertiesDelegate::updateEditorGeometry(
  QWidget* editor,
  const QStyleOptionViewItem& option,
  const QModelIndex&) const
{
  editor->setGeometry(option.rect);
}

}