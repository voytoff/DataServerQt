#pragma once

#include "datasettings_export.h"

#include <QString>

class QWidget;
class QMainWindow;
class QSplitter;
class QHeaderView;

namespace qds
{

class DATASETTINGS_EXPORT WindowSettings
{
public:
  static void saveGeometry(
    const QString& fileName,
    const QString& key,
    const QWidget* widget);

  static void restoreGeometry(
    const QString& fileName,
    const QString& key,
    QWidget* widget);

  static void saveState(
    const QString& fileName,
    const QString& key,
    const QMainWindow* window);

  static void restoreState(
    const QString& fileName,
    const QString& key,
    QMainWindow* window);

  static void saveSplitter(
    const QString& fileName,
    const QString& key,
    const QSplitter* splitter);

  static void restoreSplitter(
    const QString& fileName,
    const QString& key,
    QSplitter* splitter);

  static void saveHeader(
    const QString& fileName,
    const QString& key,
    const QHeaderView* header);

  static void restoreHeader(
    const QString& fileName,
    const QString& key,
    QHeaderView* header);
};

}