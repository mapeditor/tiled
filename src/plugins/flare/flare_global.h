#ifndef FLARE_GLOBAL_H
#define FLARE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(FLARE_LIBRARY)
#  define FLARESHARED_EXPORT Q_DECL_EXPORT
#else
#  define FLARESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // FLARE_GLOBAL_H
