#pragma once

#include <QtCore/qglobal.h>

#if defined(DATAEXPORT_LIBRARY)
#define DATAEXPORT_EXPORT Q_DECL_EXPORT
#else
#define DATAEXPORT_EXPORT Q_DECL_IMPORT
#endif
