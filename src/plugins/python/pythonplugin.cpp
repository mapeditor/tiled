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

#ifdef MS_WINDOWS
#define _WIN32_WINNT 0x0500 // for GetConsoleWindow
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#endif

using namespace Python;

QString scriptdir(QDir::homePath() + "/.tiled");
QMap<QString,PyObject*> knownExtModules;
QMap<QString,PyObject*> knownExtClasses;
QTextStream cout(stdout, QIODevice::WriteOnly);
QTextStream cerr(stderr, QIODevice::WriteOnly);
PyObject *pTiledCls;

PythonPlugin::~PythonPlugin() {
  #ifdef MS_WINDOWS
  FreeConsole();
  #endif

  Py_Finalize();
}

PythonPlugin::PythonPlugin()
{
  if(!Py_IsInitialized()) {
    Py_Initialize();
    inittiled();

#ifdef MS_WINDOWS
    AllocConsole();
    SetConsoleTitle(L"Tiled Python Debug Console");

    // disable close button
    HWND hConsole = GetConsoleWindow();
    HMENU hm = (HMENU)GetSystemMenu(hConsole,false);
    DeleteMenu(hm, SC_CLOSE, MF_BYCOMMAND);
    // disable ctrl-c
    SetConsoleCtrlHandler(NULL, TRUE);

    // more scroll buffer
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.Y = 9999;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

    freopen("CON", "wt", stdout);
    freopen("CON", "wt", stderr);
    freopen("CON", "rt", stdin);

    // oddly CONERR$ won't point to the console
    PyRun_SimpleString("import sys\n"
                       "sys.stdout = open(\"CONOUT$\", \"w\", 0)\n"
                       "sys.stderr = sys.stdout\n");
#endif

    // get reference to base class to find it's extensions later on
    PyObject *pmod = PyImport_ImportModule("tiled");
    pTiledCls = PyObject_GetAttrString(pmod, "Plugin");
    if(!pTiledCls || !PyCallable_Check(pTiledCls)) {
      cerr << "Can't find tiled.Plugin baseclass" << endl;
      Py_XDECREF(pTiledCls);
      handleError();
      return;
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
 * Call whenever there might be an error (if error state is not
 * cleared it leaks to next PyErr_Occurred check and reports error
 * in wrong place causing confusion)
 */
void PythonPlugin::handleError() const {
  if(PyErr_Occurred() != NULL) {
    PyErr_Print();
  }
}

/**
 * Finds the first python class that extends tiled.Plugin
 */
PyObject *PythonPlugin::findPluginSubclass(PyObject *pmod) {
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
PyObject *PythonPlugin::checkFunction(PyObject *pcls, const char *fun) const {
  PyObject *pfun = PyObject_GetAttrString(pcls, fun);
  if(!pfun || !PyCallable_Check(pfun)) {
    cerr << "No such function defined: " << fun << endl;
    return NULL;
  }
  return pfun;
}

/**
 * Calls supportsFile python function in the given class with
 * a filename to see if the class is willing to handle it
 */
bool PythonPlugin::checkFileSupport(PyObject* cls, char *file) const {
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
      cout << "-- Reloading " << name << endl;
      Py_XDECREF(knownExtClasses[name]);
      knownExtClasses.remove(name);

      pmod = PyImport_ReloadModule(knownExtModules[name]);

    } else {
      cout << "-- Loading " << name << endl;
      pmod = PyImport_ImportModule(name.toUtf8().data());
      knownExtModules[name] = pmod;
    }

    if(pmod == NULL) {
      cerr << "** Parse exception **" << endl;
      PyErr_Print();
      PyErr_Clear();
      Py_XDECREF(knownExtModules[name]);
      knownExtModules.remove(name);
      continue;
    }

    PyObject *pcls = findPluginSubclass(pmod);
    if(!pcls || !PyCallable_Check(pcls)) {
      cerr << "Extension of tiled.Plugin not defined in script: " << name << endl;
      Py_XDECREF(knownExtModules[name]);
      knownExtModules.remove(name);
      continue;
    }
    knownExtClasses.insert(name, pcls);
  }
}

/**
 * Implements Tiled::MapReaderInterface
 */
Tiled::Map *PythonPlugin::read(const QString &fileName)
{
  reloadModules();

  QMapIterator<QString, PyObject*> it(knownExtClasses);
  while (it.hasNext()) {
    it.next();
    if(!checkFileSupport(it.value(), fileName.toUtf8().data())) continue;
    cout << "-- " << it.key() << " supports " << fileName << endl;

    PyObject *pread = checkFunction(it.value(), "read");
    PyObject *pinst = PyEval_CallFunction(pread, "(s)", fileName.toUtf8().data());

    Tiled::Map *ret = new Tiled::Map(Tiled::Map::Orthogonal, 10,10, 16,16);
    if(!pinst) {
      cerr << "** Uncaught exception in script **" << endl;
    } else {
      _wrap_convert_py2c__Tiled__Map(pinst, ret);
      Py_DECREF(pinst);
    }
    handleError();

    Py_DECREF(pread);
    ret->setProperty("__script__", it.key());
    return ret->clone();
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

  QMapIterator<QString, PyObject*> it(knownExtClasses);
  while (it.hasNext()) {
    it.next();
    if(map->property("__script__") != it.key()) continue;
    cout << "-- Script used for exporting: " << it.key() << endl;

    PyObject *pmap = _wrap_convert_c2py__Tiled__Map_const(map->clone());
    if(!pmap) return false;
    PyObject *pwrite = checkFunction(it.value(), "write");
    if(!pwrite) return false;
    PyObject *pinst = PyEval_CallFunction(pwrite, "(Ns)", pmap, fileName.toUtf8().data());
    Py_DECREF(pwrite);

    if(!pinst) {
      cerr << "** Uncaught exception in script **" << endl;
      mError = "Uncaught exception in script. Please check console.";
    } else {
      bool ret = PyObject_IsTrue(pinst);
      Py_DECREF(pinst);
      if(!ret) mError = "Script returned false. Please check console.";
      return ret;
    }
    handleError();
    return false;
  }
  mError = "Export aborted. Map property \"__script__\" undefined or script missing";
  cout << "-- " << mError << endl;
  return false;
}

/**
 * Implements Tiled::MapReaderInterface and Tiled::MapWriterInterface
 */
QStringList PythonPlugin::nameFilters() const
{
  QStringList ret;

  QMapIterator<QString, PyObject*> it(knownExtClasses);
  while (it.hasNext()) {
    it.next();

    // find fun
    PyObject *pfun = PyObject_GetAttrString(it.value(), "nameFilter");
    if(!pfun || !PyCallable_Check(pfun)) {
      cerr << "Plugin extension doesn't define \"nameFilter\"" << endl;
      continue;
    }

    // have fun
    PyObject *pinst = PyEval_CallFunction(pfun, "()");
    if(!pinst) {
      cerr << "** Uncaught exception in script **" << endl;
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
  QMapIterator<QString, PyObject*> it(knownExtClasses);
  while (it.hasNext()) {
    it.next();
    if(checkFileSupport(it.value(), fileName.toUtf8().data())) {
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

Q_EXPORT_PLUGIN2(Python, PythonPlugin)
