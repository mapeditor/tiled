/*
    Copyright (c) 2017 Kevin Funk <kfunk@.kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#pragma once

#include <qglobal.h>

#if QT_VERSION < QT_VERSION_CHECK(5,7,0)
namespace QtPrivate
{
template <typename T> struct QAddConst {
    typedef const T Type;
};
}

// this adds const to non-const objects (like std::as_const)
template <typename T>
Q_DECL_CONSTEXPR typename QtPrivate::QAddConst<T>::Type &qAsConst(T &t) Q_DECL_NOTHROW { return t; }
// prevent rvalue arguments:
template <typename T>
void qAsConst(const T &&) Q_DECL_EQ_DELETE;
#endif

// compat for Q_FALLTHROUGH
#if QT_VERSION < QT_VERSION_CHECK(5,8,0)

#if defined(__has_cpp_attribute)
#    if __has_cpp_attribute(fallthrough)
#        define Q_FALLTHROUGH() [[fallthrough]]
#    elif __has_cpp_attribute(clang::fallthrough)
#        define Q_FALLTHROUGH() [[clang::fallthrough]]
#    elif __has_cpp_attribute(gnu::fallthrough)
#        define Q_FALLTHROUGH() [[gnu::fallthrough]]
#    endif
#endif

#ifndef Q_FALLTHROUGH
#    if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 700)
#        define Q_FALLTHROUGH() __attribute__((fallthrough))
#    else
#        define Q_FALLTHROUGH() (void)0
#    endif
#endif

#endif
