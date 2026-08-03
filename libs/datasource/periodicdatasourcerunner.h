#pragma once

#include "generatordatasource.h"
#include <QObject>
#include <QTimer>

namespace qds
{

class PeriodicDataSourceRunner : public QObject
{
  Q_OBJECT

public:
  explicit PeriodicDataSourceRunner(
    GeneratorDataSource& source,
    QObject* parent = nullptr);

  void start(int intervalMs);
  void stop();

  bool isRunning() const;

private:
  GeneratorDataSource& m_source;
  QTimer m_timer;
};

}