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
#include "tilesetformat.h"

#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>
#include <QTimer>

namespace Tiled {
class Map;
}

namespace Python {

class PythonMapFormat;
class PythonTilesetFormat;

struct ScriptEntry
{
    QString name;
    PyObject *module = nullptr;
    PythonMapFormat *mapFormat = nullptr;
    PythonTilesetFormat *tilesetFormat = nullptr;
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

    PyObject *findPluginSubclass(PyObject *module, PyObject *pluginClass);

    QString mScriptDir;
    QMap<QString,ScriptEntry> mScripts;
    PyObject *mPluginClass;
    PyObject *mTilesetPluginClass;

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

// Class exposed for Python scripts to extend
class PythonTilesetScript {
public:
    // perhaps provide default that throws NotImplementedError
    Tiled::SharedTileset *read(const QString &fileName);
    bool supportsFile(const QString &fileName) const;
    bool write(const Tiled::Tileset &tileset, const QString &fileName);
    QString nameFilter() const;
};

class PythonFormat
{
public:
    PyObject *pythonClass() const { return mClass; }
    void setPythonClass(PyObject *class_);

protected:
    PythonFormat(const QString &scriptFile, PyObject *class_);

    bool _supportsFile(const QString &fileName) const;

    QString _nameFilter() const;
    QString _shortName() const;
    QString _errorString() const;

    PyObject *mClass;
    QString mScriptFile;
    QString mError;
    Tiled::FileFormat::Capabilities mCapabilities;
};

class PythonMapFormat : public Tiled::MapFormat, public PythonFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapFormat)

public:
    PythonMapFormat(const QString &scriptFile,
                    PyObject *class_,
                    QObject *parent = nullptr);

    Capabilities capabilities() const override { return mCapabilities; };

    std::unique_ptr<Tiled::Map> read(const QString &fileName) override;
    bool supportsFile(const QString &fileName) const override { return _supportsFile(fileName); }

    bool write(const Tiled::Map *map, const QString &fileName, Options options) override;

    QString nameFilter() const override { return _nameFilter(); }
    QString shortName() const override { return _shortName(); }
    QString errorString() const override { return _errorString(); }
};

class PythonTilesetFormat : public Tiled::TilesetFormat, public PythonFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::TilesetFormat)

public:
    PythonTilesetFormat(const QString &scriptFile,
                        PyObject *class_,
                        QObject *parent = nullptr);

    Capabilities capabilities() const override { return mCapabilities; };

    Tiled::SharedTileset read(const QString &fileName) override;
    bool supportsFile(const QString &fileName) const override { return _supportsFile(fileName); }

    bool write(const Tiled::Tileset &tileset, const QString &fileName, Options options) override;

    QString nameFilter() const override { return _nameFilter(); }
    QString shortName() const override { return _shortName(); }
    QString errorString() const override { return _errorString(); }
};

} // namespace Python
