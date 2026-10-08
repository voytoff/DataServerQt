#include "configurationpropertiesdelegate.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QDoubleSpinBox>

namespace qds
{

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

  const QString property =
    index.siblingAtColumn(0)
      .data(Qt::DisplayRole)
      .toString();

  if (property == tr("Частота опроса, Гц"))
  {
    auto* editor =
      new QDoubleSpinBox(parent);

    editor->setDecimals(2);
    editor->setRange(0.01, 400000.0);
    editor->setSingleStep(100.0);
    editor->setSuffix(tr(" Гц"));

    return editor;
  }

  if (property == tr("Режим"))
  {
    auto* editor =
      new QComboBox(parent);

    editor->addItem(
      tr("Дифференциальный"),
      0);

    editor->addItem(
      tr("Общая земля"),
      1);

    editor->addItem(
      tr("Измерение нуля"),
      2);

    return editor;
  }

  if (property == tr("Диапазон"))
  {
    auto* editor =
      new QComboBox(parent);

    editor->addItem(
      tr("±10 В"),
      0);

    editor->addItem(
      tr("±2,5 В"),
      1);

    editor->addItem(
      tr("±0,625 В"),
      2);

    editor->addItem(
      tr("±0,156 В"),
      3);

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

  if (auto* combo =
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

  if (auto* combo =
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