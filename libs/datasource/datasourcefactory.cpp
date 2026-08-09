#include "datasourcefactory.h"

namespace qds
{

std::unique_ptr<IDataSource> DataSourceFactory::create(const ModuleInfo &module)
{
  // пока здесь выбор типа IDataSource
  // и создание конкретной реализации

}

}