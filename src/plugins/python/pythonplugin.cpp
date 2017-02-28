/*
 * Python Tiled Plugin
 * Copyright 2012-2013, Samuli Tuomola <samuli@tuomola.net>
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

#include "pythonplugin.h"

#include "map.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>

namespace Python {

/**
 * Call whenever there might be an error (if error state is not cleared it
 * leaks to next PyErr_Occurred check and reports error in wrong place causing
 * confusion)
 */
static void handleError()
{
    if (PyErr_Occurred() != nullptr)
        PyErr_Print();
}

PythonPlugin::PythonPlugin()
    : mScriptDir(QDir::homePath() + "/.tiled")
    , mPluginClass(nullptr)
{
    mReloadTimer.setSingleShot(true);
    mReloadTimer.setInterval(1000);

    connect(&mReloadTimer, &QTimer::timeout,
            this, &PythonPlugin::reloadModules);
}

PythonPlugin::~PythonPlugin()
{
    for (const ScriptEntry &script : mScripts) {
        Py_DECREF(script.module);
        Py_DECREF(script.mapFormat->pythonClass());
    }

    Py_XDECREF(mPluginClass);

    Py_Finalize();
}

void PythonPlugin::initialize()
{
    addObject(&mLogger);

    if (!Py_IsInitialized()) {
        // PEP370
        Py_NoSiteFlag = 1;
        Py_NoUserSiteDirectory = 1;

        Py_Initialize();
        inittiled();

        // Get reference to base class to find its extensions later on
        PyObject *pmod = PyImport_ImportModule("tiled");
        if (pmod) {
            PyObject *tiledPlugin = PyObject_GetAttrString(pmod, "Plugin");
            Py_DECREF(pmod);

            if (tiledPlugin) {
                if (PyCallable_Check(tiledPlugin)) {
                    mPluginClass = tiledPlugin;
                } else {
                    Py_DECREF(tiledPlugin);
                }
            }
        }

        if (!mPluginClass) {
            log(Tiled::LoggingInterface::ERROR, "Can't find tiled.Plugin baseclass\n");
            handleError();
            return;
        }

        // w/o differentiating error messages could just rename "log"
        // to "write" in the binding and assign plugin directly to stdout/stderr
        PySys_SetObject((char *)"_tiledplugin",
                        _wrap_convert_c2py__Tiled__LoggingInterface(&mLogger));

        PyRun_SimpleString("import sys\n"
                           "#from tiled.Tiled.LoggingInterface import INFO,ERROR\n"
                           "class _Catcher:\n"
                           "   def __init__(self, type):\n"
                           "      self.buffer = ''\n"
                           "      self.type = type\n"
                           "   def write(self, msg):\n"
                           "      self.buffer += msg\n"
                           "      if self.buffer.endswith('\\n'):\n"
                           "         sys._tiledplugin.log(self.type, self.buffer)\n"
                           "         self.buffer = ''\n"
                           "sys.stdout = _Catcher(0)\n"
                           "sys.stderr = _Catcher(1)\n");

        PyRun_SimpleString(QString("import sys; sys.path.insert(0, \"%1\")")
                           .arg(mScriptDir).toUtf8().constData());

        log(QString("-- Added %1 to path\n").arg(mScriptDir));
    }

    reloadModules();

    if (QFile::exists(mScriptDir)) {
        mFileSystemWatcher.addPath(mScriptDir);

        connect(&mFileSystemWatcher, SIGNAL(directoryChanged(QString)),
                &mReloadTimer, SLOT(start()));
    }
}

void PythonPlugin::log(Tiled::LoggingInterface::OutputType type,
                       const QString &msg)
{
    mLogger.log(type, msg);
}

void PythonPlugin::log(const QString &msg)
{
    log(Tiled::LoggingInterface::INFO, msg);
}

/**
 * (Re)load modules in the script directory
 */
void PythonPlugin::reloadModules()
{
    log(tr("Reloading Python scripts"));

    const QStringList pyfilter("*.py");
    QDirIterator iter(mScriptDir, pyfilter, QDir::Files | QDir::Readable);

    while (iter.hasNext()) {
        iter.next();

        QString name = iter.fileInfo().baseName();

        ScriptEntry script = mScripts.take(name);
        script.name = name;

        // Throw away any existing class reference
        if (script.mapFormat) {
            PyObject *pluginClass = script.mapFormat->pythonClass();
            Py_DECREF(pluginClass);
        }

        if (loadOrReloadModule(script)) {
            mScripts.insert(name, script);
        } else {
            if (!script.module) {
                PySys_WriteStderr("** Parse exception **\n");
                PyErr_Print();
                PyErr_Clear();
            }

            if (script.mapFormat) {
                removeObject(script.mapFormat);
                delete script.mapFormat;
            }
        }
    }
}

/**
 * Finds the first python class that extends tiled.Plugin
 */
PyObject *PythonPlugin::findPluginSubclass(PyObject *module)
{
    PyObject *dir = PyObject_Dir(module);
    PyObject *result = nullptr;

    for (int i = 0; i < PyList_Size(dir); i++) {
        PyObject *value = PyObject_GetAttr(module, PyList_GetItem(dir, i));

        if (!value) {
            handleError();
            break;
        }

        if (value != mPluginClass &&
                PyCallable_Check(value) &&
                PyObject_IsSubclass(value, mPluginClass) == 1) {
            result = value;
            handleError();
            break;
        }

        Py_DECREF(value);
    }

    Py_DECREF(dir);
    return result;
}

bool PythonPlugin::loadOrReloadModule(ScriptEntry &script)
{
    const QByteArray name = script.name.toUtf8();

    if (script.module) {
        PySys_WriteStdout("-- Reloading %s\n", name.constData());

        PyObject *module = PyImport_ReloadModule(script.module);
        Py_DECREF(script.module);
        script.module = module;
    } else {
        PySys_WriteStdout("-- Loading %s\n", name.constData());
        script.module = PyImport_ImportModule(name.constData());
    }

    if (!script.module)
        return false;

    PyObject *pluginClass = findPluginSubclass(script.module);

    if (!pluginClass) {
        PySys_WriteStderr("Extension of tiled.Plugin not defined in "
                          "script: %s\n", name.constData());
        return false;
    }

    if (script.mapFormat) {
        script.mapFormat->setPythonClass(pluginClass);
    } else {
        script.mapFormat = new PythonMapFormat(name, pluginClass, *this);
        addObject(script.mapFormat);
    }

    return true;
}


PythonMapFormat::PythonMapFormat(const QString &scriptFile,
                                 PyObject *class_,
                                 PythonPlugin &plugin)
    : MapFormat(&plugin)
    , mClass(nullptr)
    , mPlugin(plugin)
    , mScriptFile(scriptFile)
{
    setPythonClass(class_);
}

Tiled::Map *PythonMapFormat::read(const QString &fileName)
{
    mError = QString();

    mPlugin.log(tr("-- Using script %1 to read %2").arg(mScriptFile, fileName));

    if (!PyObject_HasAttrString(mClass, "read")) {
        mError = "Please define class that extends tiled.Plugin and "
                "has @classmethod read(cls, filename)";
        return nullptr;
    }
    PyObject *pinst = PyObject_CallMethod(mClass, (char *)"read",
                                          (char *)"(s)", fileName.toUtf8().constData());

    Tiled::Map *ret = nullptr;
    if (!pinst) {
        PySys_WriteStderr("** Uncaught exception in script **\n");
    } else {
        _wrap_convert_py2c__Tiled__Map___star__(pinst, &ret);
        Py_DECREF(pinst);
    }
    handleError();

    if (ret)
        ret->setProperty("__script__", mScriptFile);
    return ret;
}

bool PythonMapFormat::write(const Tiled::Map *map, const QString &fileName)
{
    mError = QString();

    mPlugin.log(tr("-- Using script %1 to write %2").arg(mScriptFile, fileName));

    PyObject *pmap = _wrap_convert_c2py__Tiled__Map_const(map);
    if (!pmap)
        return false;
    PyObject *pinst = PyObject_CallMethod(mClass,
                                          (char *)"write", (char *)"(Ns)",
                                          pmap,
                                          fileName.toUtf8().constData());

    if (!pinst) {
        PySys_WriteStderr("** Uncaught exception in script **\n");
        mError = tr("Uncaught exception in script. Please check console.");
    } else {
        bool ret = PyObject_IsTrue(pinst);
        Py_DECREF(pinst);
        if (!ret)
            mError = tr("Script returned false. Please check console.");
        return ret;
    }

    handleError();
    return false;
}

bool PythonMapFormat::supportsFile(const QString &fileName) const
{
    if (!PyObject_HasAttrString(mClass, "supportsFile"))
        return false;

    PyObject *pinst = PyObject_CallMethod(mClass,
                                          (char *)"supportsFile",
                                          (char *)"(s)",
                                          fileName.toUtf8().constData());
    if (!pinst) {
        handleError();
        return false;
    }

    bool ret = PyObject_IsTrue(pinst);
    Py_DECREF(pinst);
    return ret;
}

QString PythonMapFormat::nameFilter() const
{
    QString ret;

    // find fun
    PyObject *pfun = PyObject_GetAttrString(mClass, "nameFilter");
    if (!pfun || !PyCallable_Check(pfun)) {
        PySys_WriteStderr("Plugin extension doesn't define \"nameFilter\"\n");
        return ret;
    }

    // have fun
    PyObject *pinst = PyEval_CallFunction(pfun, "()");
    if (!pinst) {
        PySys_WriteStderr("** Uncaught exception in script **\n");
    } else {
        ret = PyString_AsString(pinst);
        Py_DECREF(pinst);
    }
    handleError();

    Py_DECREF(pfun);

    return ret;
}

QString PythonMapFormat::shortName() const
{
    QString ret;

    // find fun
    PyObject *pfun = PyObject_GetAttrString(mClass, "shortName");
    if (!pfun || !PyCallable_Check(pfun)) {
        PySys_WriteStderr("Plugin extension doesn't define \"nameFilter\". Falling back to \"nameFilter\"\n");
        return nameFilter();
    }

    // have fun
    PyObject *pinst = PyEval_CallFunction(pfun, "()");
    if (!pinst) {
        PySys_WriteStderr("** Uncaught exception in script **\n");
    } else {
        ret = PyString_AsString(pinst);
        Py_DECREF(pinst);
    }
    handleError();

    Py_DECREF(pfun);

    return ret;
}

QString PythonMapFormat::errorString() const
{
    return mError;
}

void PythonMapFormat::setPythonClass(PyObject *class_)
{
    mClass = class_;
    mCapabilities = NoCapability;

    // @classmethod nameFilter(cls)
    if (PyObject_HasAttrString(mClass, "nameFilter")) {
        // @classmethod write(cls, map, filename)
        if (PyObject_HasAttrString(mClass, "write")) {
            mCapabilities |= Tiled::MapFormat::Write;
        }

        // @classmethod read(cls, filename)
        // @classmethod supportsFile(cls, filename)
        if (PyObject_HasAttrString(mClass, "read") &&
                PyObject_HasAttrString(mClass, "supportsFile")) {
            mCapabilities |= Tiled::MapFormat::Read;
        }
    }
}

} // namespace Python
