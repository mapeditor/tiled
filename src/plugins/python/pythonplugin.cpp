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

#include <stdlib.h>
#include <Python.h>
#include <list>
#include <string>
#include <iostream>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDirIterator>

using namespace Python;

PythonPlugin::~PythonPlugin()
{
    foreach (PyObject *pythonClass, mKnownExtClasses)
        Py_DECREF(pythonClass);
    foreach (PyObject *pythonModule, mKnownExtModules)
        Py_DECREF(pythonModule);

    Py_XDECREF(pTiledCls);

    Py_Finalize();
}

PythonPlugin::PythonPlugin()
    : mScriptDir(QDir::homePath() + "/.tiled")
{
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
                    pTiledCls = tiledPlugin;
                } else {
                    Py_DECREF(tiledPlugin);
                }
            }
        }

        if (!pTiledCls) {
            log(ERROR, "Can't find tiled.Plugin baseclass\n");
            handleError();
            return;
        }

        // w/o differentiating error messages could just rename "log"
        // to "write" in the binding and assign plugin directly to stdout/stderr
        PySys_SetObject((char *)"_tiledplugin",
                        _wrap_convert_c2py__Tiled__LoggingInterface(this));

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
                           .arg(mScriptDir).toUtf8().data());

        log(QString("-- Added %1 to path\n").arg(mScriptDir));
    }

    reloadModules();
}

void PythonPlugin::log(OutputType type, const QString msg)
{
    if (type == INFO)
        emit info(msg);
    else if (type == ERROR)
        emit error(msg);
}

void PythonPlugin::log(const QString msg)
{
    log(INFO, msg);
}

/**
 * Call whenever there might be an error (if error state is not cleared it
 * leaks to next PyErr_Occurred check and reports error in wrong place causing
 * confusion)
 */
void PythonPlugin::handleError() const
{
    if (PyErr_Occurred() != NULL)
        PyErr_Print();
}

/**
 * Finds the first python class that extends tiled.Plugin
 */
PyObject *PythonPlugin::findPluginSubclass(PyObject *pmod)
{
    PyObject *pdir = PyObject_Dir(pmod);

    for (int i = 0; i < PyList_Size(pdir); i++) {
        PyObject *pit = PyObject_GetAttr(pmod, PyList_GetItem(pdir, i));
        if (!pit) {
            Py_DECREF(pdir);
            handleError();
            return NULL;
        }

        if (pit != pTiledCls && PyObject_IsSubclass(pit, pTiledCls) == 1) {
            PyObject *ret = PyObject_GetAttr(pmod, PyList_GetItem(pdir, i));
            Py_DECREF(pit);
            Py_DECREF(pdir);
            handleError();
            return ret;
        }
        Py_DECREF(pit);
    }

    Py_DECREF(pdir);
    return NULL;
}

/**
 * Gets a function from python class and does some validation
 */
PyObject *PythonPlugin::checkFunction(PyObject *pcls, const char *fun) const
{
    PyObject *pfun = PyObject_GetAttrString(pcls, fun);

    if (!pfun || !PyCallable_Check(pfun)) {
        PySys_WriteStderr("No such function defined: %s\n", fun);
        Py_XDECREF(pfun);
        return NULL;
    }

    return pfun;
}

/**
 * Calls supportsFile python function in the given class with a filename to see
 * if the class is willing to handle it
 */
bool PythonPlugin::checkFileSupport(PyObject *cls, char *file) const
{
    if (!PyObject_HasAttrString(cls, "supportsFile")) {
        PySys_WriteStderr("Please define class that extends tiled.Plugin "
                          "and has @classmethod supportsFile(cls, filename)\n");
        return false;
    }

    PyObject *pinst = PyObject_CallMethod(cls, (char *)"supportsFile",
                                          (char *)"(s)", file);
    if (!pinst) {
        handleError();
        return false;
    }

    bool ret = PyObject_IsTrue(pinst);
    Py_DECREF(pinst);
    return ret;
}

/**
 * (Re)load modules in the script directory
 */
void PythonPlugin::reloadModules()
{
    // try to avoid unnecessary reloading
    // TODO: mLastReload not initialized
    if (QDateTime::currentDateTime().toTime_t() - mLastReload < 10)
        return;

    mLastReload = QDateTime::currentDateTime().toTime_t();

    QStringList pyfilter("*.py");
    QDirIterator iter(mScriptDir, pyfilter, QDir::Files | QDir::Readable);

    while (iter.hasNext()) {
        iter.next();
        QString name = iter.fileInfo().baseName();
        PyObject *pmod;
        PyObject *knownModule = mKnownExtModules.take(name);

        if (knownModule) {
            PySys_WriteStdout("-- Reloading %s\n", name.toUtf8().data());
            PyObject *moduleClass = mKnownExtClasses.take(name);
            Py_XDECREF(moduleClass);

            pmod = PyImport_ReloadModule(knownModule);
            Py_DECREF(knownModule);
        } else {
            PySys_WriteStdout("-- Loading %s\n", name.toUtf8().data());
            pmod = PyImport_ImportModule(name.toUtf8().data());
        }

        if (!pmod) {
            PySys_WriteStderr("** Parse exception **\n");
            PyErr_Print();
            PyErr_Clear();
            continue;
        }

        mKnownExtModules[name] = pmod;

        PyObject *pcls = findPluginSubclass(pmod);

        if (!pcls || !PyCallable_Check(pcls)) {
            PySys_WriteStderr("Extension of tiled.Plugin not defined in "
                              "script: %s\n", name.toUtf8().data());
            Py_XDECREF(pcls);
            continue;
        }

        mKnownExtClasses.insert(name, pcls);
    }
}

/**
 * Implements Tiled::MapReaderInterface
 */
Tiled::Map *PythonPlugin::read(const QString &fileName)
{
    reloadModules();

    QMapIterator<QString, PyObject*> it(mKnownExtClasses);
    while (it.hasNext()) {
        it.next();
        if (!checkFileSupport(it.value(), fileName.toUtf8().data()))
            continue;
        log(QString("-- %1 supports %2\n").arg(it.key()).arg(fileName));

        if (!PyObject_HasAttrString(it.value(), "read")) {
            mError = "Please define class that extends tiled.Plugin and "
                    "has @classmethod read(cls, filename)";
            return NULL;
        }
        PyObject *pinst = PyObject_CallMethod(it.value(), (char *)"read",
                                              (char *)"(s)", fileName.toUtf8().data());

        Tiled::Map *ret = 0;
        if (!pinst) {
            PySys_WriteStderr("** Uncaught exception in script **\n");
        } else {
            _wrap_convert_py2c__Tiled__Map___star__(pinst, &ret);
            Py_DECREF(pinst);
        }
        handleError();

        if (ret)
            ret->setProperty("__script__", it.key());
        return ret;
    }
    return NULL;
}

/**
 * Implements Tiled::MapWriterInterface
 */
bool PythonPlugin::write(const Tiled::Map *map, const QString &fileName)
{
    reloadModules();
    mError = "";

    QMapIterator<QString, PyObject*> it(mKnownExtClasses);
    while (it.hasNext()) {
        it.next();
        if (map->property("__script__") != it.key()) continue;
        log(QString("-- Script used for exporting: %1\n").arg(it.key()));

        PyObject *pmap = _wrap_convert_c2py__Tiled__Map_const(map);
        if (!pmap) return false;
        if (!PyObject_HasAttrString(it.value(), "write")) {
            mError = "Please define class that extends tiled.Plugin and has "
                    "@classmethod write(cls, map, filename)";
            return NULL;
        }
        PyObject *pinst = PyObject_CallMethod(it.value(), (char *)"write",
                                              (char *)"(Ns)", pmap, fileName.toUtf8().data());

        if (!pinst) {
            PySys_WriteStderr("** Uncaught exception in script **\n");
            mError = "Uncaught exception in script. Please check console.";
        } else {
            bool ret = PyObject_IsTrue(pinst);
            Py_DECREF(pinst);
            if (!ret) mError = "Script returned false. Please check console.";
            return ret;
        }
        handleError();
        return false;
    }
    mError = "Export aborted. Map property \"__script__\" undefined or script missing";
    PySys_WriteStderr("%s.1000s", mError.append('\n').toUtf8().data());
    return false;
}

/**
 * Implements Tiled::MapReaderInterface and Tiled::MapWriterInterface
 */
QStringList PythonPlugin::nameFilters() const
{
    QStringList ret;

    QMapIterator<QString, PyObject*> it(mKnownExtClasses);
    while (it.hasNext()) {
        it.next();

        // find fun
        PyObject *pfun = PyObject_GetAttrString(it.value(), "nameFilter");
        if (!pfun || !PyCallable_Check(pfun)) {
            PySys_WriteStderr("Plugin extension doesn't define \"nameFilter\"\n");
            continue;
        }

        // have fun
        PyObject *pinst = PyEval_CallFunction(pfun, "()");
        if (!pinst) {
            PySys_WriteStderr("** Uncaught exception in script **\n");
        } else {
            ret += PyString_AsString(pinst);
            Py_DECREF(pinst);
        }
        handleError();

        Py_DECREF(pfun);
    }

    return ret;
}

/**
 * Implements Tiled::MapReaderInterface
 */
bool PythonPlugin::supportsFile(const QString &fileName) const
{
    QMapIterator<QString, PyObject*> it(mKnownExtClasses);
    while (it.hasNext()) {
        it.next();
        if (checkFileSupport(it.value(), fileName.toUtf8().data())) {
            return true;
        }
    }
    return false;
}

/**
 * Implements Tiled::MapReaderInterface and Tiled::MapWriterInterface
 */
QString PythonPlugin::errorString() const
{
    return mError;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Python, PythonPlugin)
#endif
