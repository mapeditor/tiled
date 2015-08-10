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

#ifdef __MINGW32__
#include <cmath> // included before Python.h to fix ::hypot not declared issue
#endif

#include <Python.h>

#include "logginginterface.h"
#include "mapformat.h"
#include "plugin.h"

#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>
#include <QTimer>

namespace Tiled {
class Map;
}

namespace Python {

class PythonMapFormat;

struct ScriptEntry
{
    ScriptEntry()
        : module(nullptr)
        , mapFormat(nullptr)
    {}

    QString name;
    PyObject *module;
    PythonMapFormat *mapFormat;
};

class Q_DECL_EXPORT PythonPlugin : public Tiled::Plugin
{
    Q_OBJECT
    Q_INTERFACES(Tiled::Plugin)
    Q_PLUGIN_METADATA(IID "org.mapeditor.Plugin" FILE "plugin.json")

public:
    PythonPlugin();
    ~PythonPlugin();

    void initialize() override;

    void log(Tiled::LoggingInterface::OutputType type, const QString &msg);
    void log(const QString &msg);

private slots:
    void reloadModules();

private:
    bool loadOrReloadModule(ScriptEntry &script);
    PyObject *findPluginSubclass(PyObject *module);

    QString mScriptDir;
    QMap<QString,ScriptEntry> mScripts;
    PyObject *mPluginClass;

    QFileSystemWatcher mFileSystemWatcher;
    QTimer mReloadTimer;

    Tiled::LoggingInterface mLogger;
};


// Class exposed for python scripts to extend
class PythonScript {
public:
    // perhaps provide default that throws NotImplementedError
    Tiled::Map *read(const QString &fileName);
    bool supportsFile(const QString &fileName) const;
    bool write(const Tiled::Map *map, const QString &fileName);
    QString nameFilter() const;
};


class PythonMapFormat : public Tiled::MapFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapFormat)

public:
    PythonMapFormat(const QString &scriptFile,
                    PyObject *class_,
                    PythonPlugin &plugin);

    Capabilities capabilities() const override { return mCapabilities; }

    Tiled::Map *read(const QString &fileName) override;
    bool supportsFile(const QString &fileName) const override;

    bool write(const Tiled::Map *map, const QString &fileName) override;

    QString nameFilter() const override;
    QString errorString() const override;

    PyObject *pythonClass() const { return mClass; }
    void setPythonClass(PyObject *class_);

private:
    PyObject *mClass;
    PythonPlugin &mPlugin;
    QString mScriptFile;
    QString mError;
    Capabilities mCapabilities;
};

} // namespace Python

PyMODINIT_FUNC inittiled(void);
extern int _wrap_convert_py2c__Tiled__Map___star__(PyObject *obj, Tiled::Map * *address);
extern PyObject* _wrap_convert_c2py__Tiled__Map_const(Tiled::Map const *cvalue);
extern PyObject* _wrap_convert_c2py__Tiled__LoggingInterface(Tiled::LoggingInterface *cvalue);

#endif // PYTHONPLUGIN_H
