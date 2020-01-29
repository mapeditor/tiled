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

#pragma once

#ifdef __MINGW32__
#include <cmath> // included before Python.h to fix ::hypot not declared issue
#endif

#include <Python.h>

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
    ~PythonPlugin() override;

    void initialize() override;

private:
    void reloadModules();
    bool loadOrReloadModule(ScriptEntry &script);

    PyObject *findPluginSubclass(PyObject *module);

    QString mScriptDir;
    QMap<QString,ScriptEntry> mScripts;
    PyObject *mPluginClass;

    QFileSystemWatcher mFileSystemWatcher;
    QTimer mReloadTimer;
};


// Class exposed for Python scripts to extend
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
                    QObject *parent = nullptr);

    Capabilities capabilities() const override { return mCapabilities; }

    std::unique_ptr<Tiled::Map> read(const QString &fileName) override;
    bool supportsFile(const QString &fileName) const override;

    bool write(const Tiled::Map *map, const QString &fileName, Options options) override;

    QString nameFilter() const override;
    QString shortName() const override;
    QString errorString() const override;

    PyObject *pythonClass() const { return mClass; }
    void setPythonClass(PyObject *class_);

private:
    PyObject *mClass;
    QString mScriptFile;
    QString mError;
    Capabilities mCapabilities;
};

} // namespace Python
