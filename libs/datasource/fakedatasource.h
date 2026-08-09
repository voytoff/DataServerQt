#pragma once

#include "idatasource.h"
#include <QtCore/qjsonobject.h>

namespace qds
{

class FakeDataSource : public IDataSource
{
public:
  FakeDataSource();
  explicit FakeDataSource(const QJsonObject& settings);

  bool acquire(RawMemory& memory) override;

  const QJsonObject m_settings;
};

}
