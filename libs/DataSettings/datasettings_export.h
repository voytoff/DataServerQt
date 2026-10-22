#pragma once

#include <QtCore/qglobal.h>

#if defined(DATASETTINGS_LIBRARY)
#define DATASETTINGS_EXPORT Q_DECL_EXPORT
#else
#define DATASETTINGS_EXPORT Q_DECL_IMPORT
#endif

