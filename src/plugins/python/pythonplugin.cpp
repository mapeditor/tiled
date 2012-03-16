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
#include <QMap>
#include <QTextStream>
#include <QDirIterator>

using namespace Python;

QString scriptdir(QDir::homePath() + "/.tiled");
QMap<QString,PyObject*> knownExtModules;
QMap<QString,PyObject*> knownExtClasses;
QTextStream cout(stdout, QIODevice::WriteOnly);
QTextStream cerr(stderr, QIODevice::WriteOnly);
PyObject *pTiledCls;

void handleError() {
  if(PyErr_Occurred() != NULL) {
    PyErr_Print();
    PyErr_Clear();
  }
}

PythonPlugin::PythonPlugin()
{
  if(!Py_IsInitialized()) {
    Py_Initialize();
    inittiled();

    // get reference to base class to find it's extensions later on
    PyObject *pmod = PyImport_ImportModule("tiled");
    pTiledCls = PyObject_GetAttrString(pmod, "Plugin");
    if(!pTiledCls || !PyCallable_Check(pTiledCls)) {
      cerr << "Can't find tiled.Plugin baseclass" << endl << flush;
      Py_XDECREF(pTiledCls);
      handleError();
      //exception?
    }
    Py_XDECREF(pmod);

    // simpler than doing it with PyList_*
    PyRun_SimpleString(QString("import sys; sys.path.insert(0, \"%1\")")
        .arg(scriptdir).toUtf8().data());
    cout << "-- Added " << scriptdir << " to path\n";
  }

  reloadModules();
}

/**
 * Finds the first python class that extends tiled.Plugin in
 * the current execution frame
 */
PyObject *findPluginSubclass(PyObject *pmod) {
  int i;
  PyObject *pdir = PyObject_Dir(pmod);
  for(i=0; i<PyList_Size(pdir); i++) {
    PyObject *pit = PyObject_GetAttr(pmod, PyList_GetItem(pdir, i));
    if(!pit) {
      Py_DECREF(pdir);
      handleError();
      return NULL;
    }
    //printf("testing %i %s\n", i, PyString_AsString(PyList_GetItem(pdir, i)));
    if(pit != pTiledCls && PyObject_IsSubclass(pit, pTiledCls) == 1) {
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
PyObject *checkFunction(PyObject *pcls, const char *fun) {
  PyObject *pfun = PyObject_GetAttrString(pcls, fun);
  if(!pfun || !PyCallable_Check(pfun)) {
    cerr << "No such function defined: " << fun << endl << flush;
    return NULL;
  }
  return pfun;
}

bool checkFileSupport(PyObject* cls, char *file) {
  PyObject *psupports = checkFunction(cls, "supportsFile");
  if (!psupports) return false;

  PyObject *pinst = PyEval_CallFunction(psupports, "(s)", file);
  Py_DECREF(psupports);
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
void PythonPlugin::reloadModules() {
  // try to avoid unnecessary reloading
  if(QDateTime::currentDateTime().toTime_t() - this->lastReload < 10) return;
  this->lastReload = QDateTime::currentDateTime().toTime_t();

  QStringList pyfilter("*.py");
  QDirIterator iter(scriptdir, pyfilter, QDir::Files | QDir::Readable);

  while(iter.hasNext()) {
    iter.next();
    QString name = iter.fileInfo().baseName();
    PyObject *pmod;

    if (knownExtModules.contains(name)) {
      cout << "-- Reloading " << name << endl << flush;
      Py_XDECREF(knownExtClasses[name]);
      knownExtClasses.remove(name);

      pmod = PyImport_ReloadModule(knownExtModules[name]);

    } else {
      cout << "-- Loading " << name << endl << flush;
      pmod = PyImport_ImportModule(name.toUtf8().data());
      knownExtModules[name] = pmod;
    }

    if(pmod == NULL) {
      cerr << "** Parse exception **" << endl << flush;
      PyErr_Print();
      PyErr_Clear();
      Py_XDECREF(knownExtModules[name]);
      knownExtModules.remove(name);
      continue;
    }

    PyObject *pcls = findPluginSubclass(pmod);
    if(!pcls || !PyCallable_Check(pcls)) {
      cerr << "Extension of tiled.Plugin not defined in script: " << name << endl << flush;
      Py_XDECREF(knownExtModules[name]);
      knownExtModules.remove(name);
      continue;
    }
    knownExtClasses.insert(name, pcls);
  }
}

Tiled::Map *PythonPlugin::read(const QString &fileName)
{
  reloadModules();

  QMapIterator<QString, PyObject*> it(knownExtClasses);
  while (it.hasNext()) {
    it.next();
    if(!checkFileSupport(it.value(), fileName.toUtf8().data())) continue;
    cout << "-- " << it.key() << " supports " << fileName << endl << flush;

    PyObject *pread = checkFunction(it.value(), "read");
    PyObject *pinst = PyEval_CallFunction(pread, "(s)", fileName.toUtf8().data());

    Tiled::Map *ret = new Tiled::Map(Tiled::Map::Orthogonal, 10,10, 16,16);
    if(!pinst) {
      cerr << "** Uncaught exception in script **" << endl << flush;
    } else {
      _wrap_convert_py2c__Tiled__Map(pinst, ret);
      Py_DECREF(pinst);
    }
    handleError();

    Py_DECREF(pread);
    return ret->clone();
  }
  return NULL;
}

bool PythonPlugin::write(const Tiled::Map *map, const QString &fileName)
{
  reloadModules();

  QMapIterator<QString, PyObject*> it(knownExtClasses);
  while (it.hasNext()) {
    it.next();
    if(!checkFileSupport(it.value(), fileName.toUtf8().data())) continue;
    cout << "-- " << it.key() << " supports " << fileName << endl << flush;

    PyObject *pwrite = checkFunction(it.value(), "write");
    PyObject *pmap = _wrap_convert_c2py__Tiled__Map_const(map);
    PyObject *pinst = PyEval_CallFunction(pwrite, "(Ns)", pmap, fileName.toUtf8().data());
    Py_DECREF(pmap);

    if(!pinst) {
      cerr << "** Uncaught exception in script **" << endl << flush;
    } else {
      Py_DECREF(pinst);
    }
    handleError();

    Py_DECREF(pwrite);
    return true;
  }
  return false;
}

QStringList PythonPlugin::nameFilters() const
{
  QStringList ret;

  QMapIterator<QString, PyObject*> it(knownExtClasses);
  while (it.hasNext()) {
    it.next();
    // find fun
    PyObject *pfun = PyObject_GetAttrString(it.value(), "nameFilter");
    if(!pfun || !PyCallable_Check(pfun)) {
      cerr << "Plugin extension doesn't define \"nameFilter\"" << endl << flush;
      continue;
    }
    // have fun
    PyObject *pinst = PyEval_CallFunction(pfun, "()");
    if(!pinst) {
      cerr << "** Uncaught exception in script **" << endl << flush;
      PyErr_Print();
      PyErr_Clear();
    } else {
      ret += PyString_AsString(pinst);
      Py_DECREF(pinst);
    }
    Py_DECREF(pfun);
  }

  return ret;
}

bool PythonPlugin::supportsFile(const QString &fileName) const
{
  QMapIterator<QString, PyObject*> it(knownExtClasses);
  while (it.hasNext()) {
    it.next();
    if(checkFileSupport(it.value(), fileName.toUtf8().data())) {
      return true;
    }
  }
  return false;
}

QString PythonPlugin::errorString() const
{
    return mError;
}

Q_EXPORT_PLUGIN2(Python, PythonPlugin)
