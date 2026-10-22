#pragma once

#include <QtCore/qglobal.h>

#if defined(DATAWIDGETS_LIBRARY)
#define DATAWIDGETS_EXPORT Q_DECL_EXPORT
#else
#define DATAWIDGETS_EXPORT Q_DECL_IMPORT
#endif

