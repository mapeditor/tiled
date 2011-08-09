#ifndef OSARE_GLOBAL_H
#define OSARE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(OSARE_LIBRARY)
#  define OSARESHARED_EXPORT Q_DECL_EXPORT
#else
#  define OSARESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // OSARE_GLOBAL_H
