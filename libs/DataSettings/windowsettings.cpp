#include "windowsettings.h"

#include <QWidget>
#include <QMainWindow>
#include <QSplitter>
#include <QHeaderView>
#include <QSettings>

namespace qds
{

namespace
{

void save(
  const QString& fileName,
  const QString& key,
  const QByteArray& state)
{
  QSettings settings(
    fileName,
    QSettings::IniFormat);

  settings.setValue(key, state);
}

QByteArray load(
  const QString& fileName,
  const QString& key)
{
  QSettings settings(
    fileName,
    QSettings::IniFormat);

  return settings.value(key).toByteArray();
}

}

void WindowSettings::saveGeometry(
  const QString& fileName,
  const QString& key,
  const QWidget* widget)
{
  if (widget)
    save(fileName, key, widget->saveGeometry());
}

void WindowSettings::restoreGeometry(
  const QString& fileName,
  const QString& key,
  QWidget* widget)
{
  if (!widget)
    return;

  const QByteArray state = load(fileName, key);

  if (!state.isEmpty())
    widget->restoreGeometry(state);
}

void WindowSettings::saveState(
  const QString& fileName,
  const QString& key,
  const QMainWindow* window)
{
  if (window)
    save(fileName, key, window->saveState());
}

void WindowSettings::restoreState(
  const QString& fileName,
  const QString& key,
  QMainWindow* window)
{
  if (!window)
    return;

  const QByteArray state = load(fileName, key);

  if (!state.isEmpty())
    window->restoreState(state);
}

void WindowSettings::saveSplitter(
  const QString& fileName,
  const QString& key,
  const QSplitter* splitter)
{
  if (splitter)
    save(fileName, key, splitter->saveState());
}

void WindowSettings::restoreSplitter(
  const QString& fileName,
  const QString& key,
  QSplitter* splitter)
{
  if (!splitter)
    return;

  const QByteArray state = load(fileName, key);

  if (!state.isEmpty())
    splitter->restoreState(state);
}

void WindowSettings::saveHeader(
  const QString& fileName,
  const QString& key,
  const QHeaderView* header)
{
  if (header)
    save(fileName, key, header->saveState());
}

void WindowSettings::restoreHeader(
  const QString& fileName,
  const QString& key,
  QHeaderView* header)
{
  if (!header)
    return;

  const QByteArray state = load(fileName, key);

  if (!state.isEmpty())
    header->restoreState(state);
}

}