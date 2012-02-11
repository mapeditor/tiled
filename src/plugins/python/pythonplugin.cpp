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

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDirIterator>

using namespace Python;

std::list<t_pyscript> scripts;
t_pyscript *current_script = NULL;

extern Tiled::Map *python_readCb(const char *fn, void *p);
extern bool python_writeCb(const Tiled::Map *map, const char *fn, void *p);
extern bool python_supportsCb(const char *fn, void *p);

PythonPlugin::PythonPlugin()
{
    std::string sdir(getenv("HOME"));
    sdir.append("/.tiled/");

    if(!Py_IsInitialized()) {
      Py_Initialize();
    }

    QDirIterator iterator(QString(sdir.c_str()), QDir::Files | QDir::Readable);
    if(!iterator.hasNext()) {
      //QTextStream(&mError) << "Error " << errno << ": Unable to load python scripts from " << sdir;
      mError = tr("Error: No scripts available at %1").arg(sdir.c_str());
      std::cerr << mError.toStdString() << std::endl;
      return;
    } else {
      std::cout << "Loading python scripts from " << sdir << std::endl;
    }

    while (iterator.hasNext()) {
      const QString &scriptFile = iterator.next();
      const std::string path = scriptFile.toStdString();

      if(path.size() > 3 && path.compare(path.size()-3, 3, ".py") == 0) {
        t_pyscript script = {0};	// init callback pointers
        strcpy(script.filename, path.c_str());
        script.interpreter = Py_NewInterpreter();
        current_script = &script;
        inittiled();

        int ret = PyRun_SimpleFileEx(fopen(script.filename, "r"), script.filename, true);

        if(ret == 0) {
          std::cout << "Loaded " << path << std::endl;
        } else {
          if(PyErr_Occurred() != NULL) {
            //PyerrorHandler();
          }
          Py_EndInterpreter(script.interpreter);
          std::cerr << "Failed to load " << path << ", ret=" << ret << std::endl;
          continue;
        }

        scripts.push_back(script);
      }
    }
}
/*
PythonPlugin::~PythonPlugin()
{
	if(Py_IsInitialized()) {
		Py_Finalize();
	}
}
*/
Tiled::Map *PythonPlugin::read(const QString &fileName)
{
  for (std::list<t_pyscript>::iterator itr = scripts.begin(); itr != scripts.end(); ++itr) {
    current_script = &(*itr);
    PyThreadState_Swap(current_script->interpreter);
    if(current_script->supports_cb != NULL && python_supportsCb(fileName.toStdString().c_str(), current_script->supports_cb)) {
      return python_readCb(fileName.toStdString().c_str(), current_script->read_cb);
    }
  }
  return NULL;
}

bool PythonPlugin::write(const Tiled::Map *map, const QString &fileName)
{
  for (std::list<t_pyscript>::iterator itr = scripts.begin(); itr != scripts.end(); ++itr) {
    current_script = &(*itr);
    PyThreadState_Swap(current_script->interpreter);
    return python_writeCb(map, fileName.toStdString().c_str(), current_script->write_cb);
  }
  return false;
}

QString PythonPlugin::nameFilter() const
{
  std::string ret;
  for (std::list<t_pyscript>::iterator itr = scripts.begin(); itr != scripts.end(); ++itr) {
    current_script = &(*itr);
    PyThreadState_Swap(current_script->interpreter);
    ret.append(current_script->namefilt);
  }
  return tr(ret.c_str());
}

void
python_register_nameFilter(char *filt)
{
  sprintf(current_script->namefilt, "%s", filt);
}

void python_register_read_cb(void *f) { current_script->read_cb = f; }
void python_register_write_cb(void *f) { current_script->write_cb = f; }
void python_register_supports_cb(void *f) { current_script->supports_cb = f; }

bool PythonPlugin::supportsFile(const QString &fileName) const
{
  for (std::list<t_pyscript>::iterator itr = scripts.begin(); itr != scripts.end(); ++itr) {
    current_script = &(*itr);
    PyThreadState_Swap(current_script->interpreter);
    if(current_script->supports_cb != NULL && python_supportsCb(fileName.toStdString().c_str(), current_script->supports_cb)) return true;
  }
  return false;
}

QString PythonPlugin::errorString() const
{
    return mError;
}

Q_EXPORT_PLUGIN2(Python, PythonPlugin)

