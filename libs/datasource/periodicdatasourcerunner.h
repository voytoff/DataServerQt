#ifndef PERIODICDATASOURCERUNNER_H
#define PERIODICDATASOURCERUNNER_H

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

#endif // PERIODICDATASOURCERUNNER_H
