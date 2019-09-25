/*
 * pytiled.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <Python.h>

#include <QString>
#include <QtCore/qglobal.h>

#if defined(PYTILED_LIBRARY)
#  define PYTILEDSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PYTILEDSHARED_EXPORT Q_DECL_IMPORT
#endif

namespace Tiled {
class LoggingInterface;
class Map;
}

PyMODINIT_FUNC PyInit_tiled(void);
PYTILEDSHARED_EXPORT int _wrap_convert_py2c__Tiled__Map___star__(PyObject *obj, Tiled::Map * *address);
PYTILEDSHARED_EXPORT PyObject* _wrap_convert_c2py__Tiled__Map_const___star__(Tiled::Map const * *cvalue);
PYTILEDSHARED_EXPORT PyObject* _wrap_convert_c2py__Tiled__LoggingInterface(Tiled::LoggingInterface *cvalue);

namespace Python {

// Class exposed for Python scripts to extend
class PythonScript {
public:
    // perhaps provide default that throws NotImplementedError
    Tiled::Map *read(const QString &fileName);
    bool supportsFile(const QString &fileName) const;
    bool write(const Tiled::Map *map, const QString &fileName);
    QString nameFilter() const;
};

} // namespace Python
