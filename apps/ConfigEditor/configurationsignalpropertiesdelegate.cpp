#include "configurationsignalpropertiesdelegate.h"
#include "configurationsignalpropertiesmodel.h"

#include <QLineEdit>
#include <QComboBox>
#include <QString>

namespace qds
{

using Model = ConfigurationSignalPropertiesModel;

ConfigurationSignalPropertiesDelegate::
  ConfigurationSignalPropertiesDelegate(
    QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget *ConfigurationSignalPropertiesDelegate::createEditor(
  QWidget *parent,
  const QStyleOptionViewItem &option,
  const QModelIndex &index) const
{
  if (!index.isValid() || index.column() != 1)
    return nullptr;

  const auto propertyId =
    static_cast<Model::PropertyId>(
      index.data(Model::PropertyIdRole).toInt());

  if (propertyId == Model::PropertyId::Name)
  {
    auto* editor =
      new QLineEdit(parent);

    return editor;
  }
  else if (propertyId == Model::PropertyId::ArchiveRate)
  {
    auto* editor = new QComboBox(parent);

    editor->setAutoFillBackground(true);
    editor->setBackgroundRole(QPalette::Base);

    editor->addItem(tr("1 Гц"), 1);
    editor->addItem(tr("10 Гц"), 10);
    editor->addItem(tr("100 Гц"), 100);
    editor->addItem(tr("1000 Гц"), 1000);

    return editor;
  }
  else if (propertyId == Model::PropertyId::CalibrationMode)
  {
    auto* editor = new QComboBox(parent);

    editor->setAutoFillBackground(true);
    editor->setBackgroundRole(QPalette::Base);

    editor->addItem(tr("Нет"), 0);
    editor->addItem(tr("По сигналу"), 1);
    editor->addItem(tr("По типу сигнала"), 2);

    return editor;
  }
  else if (propertyId == Model::PropertyId::SignalType)
  {
    auto* editor = new QComboBox(parent);

    editor->setAutoFillBackground(true);
    editor->setBackgroundRole(QPalette::Base);

    editor->addItem(tr("Нет"), 0);

    for (const auto& typeInfo : m_signalTypes)
    {
      editor->addItem(
        QString::fromStdString(typeInfo.name),
        typeInfo.id.value);
    }

    return editor;
  }
  else if (propertyId == Model::PropertyId::Source)
  {
    auto* editor = new QComboBox(parent);

    editor->setAutoFillBackground(true);
    editor->setBackgroundRole(QPalette::Base);

    editor->addItem(tr("Нет"), 0);

    for (const auto& typeInfo : m_tagSources)
    {
      editor->addItem(
        QString::fromStdString(typeInfo.name),
        typeInfo.id.value);
    }

    return editor;
  }
  else if (propertyId == Model::PropertyId::Formula)
  {
    auto* editor =
      new QLineEdit(parent);

    return editor;
  }

  return QStyledItemDelegate::createEditor(
    parent,
    option,
    index);
}

void ConfigurationSignalPropertiesDelegate::setEditorData(
  QWidget *editor,
  const QModelIndex &index) const
{
  const QVariant value =
    index.data(Qt::EditRole);

  if (auto* line =
      qobject_cast<QLineEdit*>(editor))
  {
    line->setText(value.toString());
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

void ConfigurationSignalPropertiesDelegate::setModelData(
  QWidget *editor,
  QAbstractItemModel *model,
  const QModelIndex &index) const
{
  if (auto* line =
      qobject_cast<QLineEdit*>(editor))
  {
    model->setData(
      index,
      line->text(),
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

void ConfigurationSignalPropertiesDelegate::updateEditorGeometry(
  QWidget *editor,
  const QStyleOptionViewItem &option,
  const QModelIndex &) const
{
  editor->setGeometry(option.rect);
}

void ConfigurationSignalPropertiesDelegate::setTagSources(
  std::vector<TagSourceInfo> sources)
{
  m_tagSources = std::move(sources);
}

void ConfigurationSignalPropertiesDelegate::setSignalTypes(
  std::vector<SignalTypeInfo> types)
{
  m_signalTypes = std::move(types);
}

}
