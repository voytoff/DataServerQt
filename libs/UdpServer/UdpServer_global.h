#ifndef UDPSERVER_GLOBAL_H
#define UDPSERVER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(UDPSERVER_LIBRARY)
#define UDPSERVER_EXPORT Q_DECL_EXPORT
#else
#define UDPSERVER_EXPORT Q_DECL_IMPORT
#endif

#endif // UDPSERVER_GLOBAL_H
