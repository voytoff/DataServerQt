#ifndef UDPSENDER_GLOBAL_H
#define UDPSENDER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(UDPSENDER_LIBRARY)
#define UDPSENDER_EXPORT Q_DECL_EXPORT
#else
#define UDPSENDER_EXPORT Q_DECL_IMPORT
#endif

#endif // UDPSENDER_GLOBAL_H
