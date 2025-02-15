#ifndef RPD_GLOBAL_H
#define RPD_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(RPD_LIBRARY)
#  define RPDSHARED_EXPORT Q_DECL_EXPORT
#else
#  define RPDSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // RPD_GLOBAL_H
