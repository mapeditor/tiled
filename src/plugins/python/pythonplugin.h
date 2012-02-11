/*
 * Python Tiled Plugin
 * Copyright 2012, Samuli Tuomola <samuli@tuomola.net>
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

#ifndef PYTHONPLUGIN_H
#define PYTHONPLUGIN_H

#include <QtCore/qglobal.h>

#include "mapwriterinterface.h"
#include "mapreaderinterface.h"

#include <QObject>
#include <Python.h>

namespace Tiled {
class Map;
}

namespace Python {

class Q_DECL_EXPORT PythonPlugin
        : public QObject
        , public Tiled::MapReaderInterface
        , public Tiled::MapWriterInterface
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapReaderInterface Tiled::MapWriterInterface)

public:
    PythonPlugin();

    // MapReaderInterface
    Tiled::Map *read(const QString &fileName);
    bool supportsFile(const QString &fileName) const;

    // MapWriterInterface
    bool write(const Tiled::Map *map, const QString &fileName);

    // Both interfaces
    QString nameFilter() const;
    QString errorString() const;

private:
    QString mError;
};

} // namespace Python

typedef struct t_pyscript {
	char filename[FILENAME_MAX];
	PyThreadState *interpreter;
  char namefilt[FILENAME_MAX];
  void *supports_cb, *read_cb, *write_cb;
} t_pyscript;

PyMODINIT_FUNC inittiled(void);
extern void python_register_nameFilter(char *filt);
extern void python_register_read_cb(void *f);
extern void python_register_write_cb(void *f);
extern void python_register_supports_cb(void *f);

#endif // PYTHONPLUGIN_H
