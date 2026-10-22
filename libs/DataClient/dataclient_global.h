#pragma once

#include <QtCore/qglobal.h>

#if defined(DATACLIENT_LIBRARY)
#define DATACLIENT_EXPORT Q_DECL_EXPORT
#else
#define DATACLIENT_EXPORT Q_DECL_IMPORT
#endif

