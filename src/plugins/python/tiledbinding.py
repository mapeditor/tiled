"""
 Python Tiled Plugin
 Copyright 2012, Samuli Tuomola <samuli@tuomola.net>

 This file is part of Tiled.

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 more details.

 You should have received a copy of the GNU General Public License along with
 this program. If not, see <http://www.gnu.org/licenses/>.
"""

from pybindgen import *
from pybindgen.typehandlers.base import ForwardWrapperBase

class PyCallbackParam(Parameter):
  DIRECTIONS = [Parameter.DIRECTION_IN]
  CTYPES = ['PyCallback']
  CALLBACK = None

  def __init__(self, ctype, name, direction=Parameter.DIRECTION_IN, is_const=False, default_value=None, callback=None):
    self.CALLBACK = callback
    super(PyCallbackParam, self).__init__(ctype, name, direction, is_const, default_value)

  def convert_python_to_c(self, wrapper):
    assert isinstance(wrapper, ForwardWrapperBase)
    #assert self.CALLBACK != None

    py_cb = wrapper.declarations.declare_variable("PyObject*", self.name)
    wrapper.parse_params.add_parameter('O', ['&'+py_cb], self.name)

    wrapper.before_call.write_error_check("!PyCallable_Check(%s)" % py_cb,
            """PyErr_SetString(PyExc_TypeError, "visitor parameter must be callable");""")
    if self.CALLBACK != None:
            wrapper.call_params.append(self.CALLBACK)
    wrapper.before_call.write_code("Py_INCREF(%s);" % py_cb)
    wrapper.before_call.add_cleanup_code("Py_DECREF(%s);" % py_cb)
    wrapper.call_params.append(py_cb)

  def convert_c_to_python(self, wrapper):
    raise NotImplementedError

class QStringParam(Parameter):
  DIRECTIONS = [Parameter.DIRECTION_IN]
  CTYPES = ['QString']
    
  def get_c_error_return(self):
    return "return QString();"

  def convert_c_to_python(self, wrapper):
    wrapper.build_params.add_parameter("s", ['%s.toUtf8().data()' % self.value], prepend=True)

  def convert_python_to_c(self, wrapper):
    name = wrapper.declarations.declare_variable("const char *", self.name)
    wrapper.parse_params.add_parameter('s', ['&'+name], self.value)
    wrapper.call_params.append('QString::fromUtf8(%s)' % name)
"""
        assert isinstance(wrapper, ForwardWrapperBase)
        if self.default_value is None:
            name = wrapper.declarations.declare_variable("const char *", self.name)
            name_len = wrapper.declarations.declare_variable("Py_ssize_t", self.name+'_len')
            wrapper.parse_params.add_parameter('s#', ['&'+name, '&'+name_len], self.value)
            wrapper.call_params.append('std::string(%s, %s)' % (name, name_len))
        else:
            name = wrapper.declarations.declare_variable("const char *", self.name, 'NULL')
            name_len = wrapper.declarations.declare_variable("Py_ssize_t", self.name+'_len')
            wrapper.parse_params.add_parameter('s#', ['&'+name, '&'+name_len], self.value, optional=True)
            wrapper.call_params.append('(%s ? std::string(%s, %s) : %s)'
                                       % (name, name, name_len, self.default_value))
"""

class QStringReturnValue(ReturnValue):
  CTYPES = ['QString']

  def get_c_error_return(self):
    return "return QString();"

  def convert_python_to_c(self, wrapper):
    raise NotImplementedError # TODO (needed only for virtual methods where C calls Python code)

  def convert_c_to_python(self, wrapper):
    wrapper.build_params.add_parameter("s", ['%s.toUtf8().data()' % self.value], prepend=True)

mod = Module('tiled')
mod.add_include('"pythonplugin.h"')
mod.add_include('"map.h"')
mod.add_include('"layer.h"')
mod.add_include('"tile.h"')
mod.add_include('"mapobject.h"')
mod.add_include('"tilelayer.h"')
mod.add_include('"objectgroup.h"')
mod.add_include('"tileset.h"')
mod.add_include('<QImage>')

tiled = mod.add_cpp_namespace('Tiled')

klass = tiled.add_class('Tile')
#klass.add_constructor([param('QImage','image'), param('int','id'), param('Tileset*','ts')])
klass.add_method('id', 'int', [])
#klass.add_method('image', retval('QPixmap&'), [])
#klass.add_method('setImage', None, [('QPixmap&','image')])
klass.add_method('width', 'int', [])
klass.add_method('height', 'int', [])
#klass.add_method('size', 'QSize', [])
cls_tile = klass

klass = tiled.add_class('Tileset')
klass.add_constructor([param('QString','name'), param('int','tw'), param('int','th'), param('int','ts'), param('int','margin')])
klass.add_method('name', 'QString', [])
klass.add_method('setName', None, [('QString','name')])
klass.add_method('fileName', 'QString', [])
klass.add_method('setFileName', None, [('QString','name')])
klass.add_method('isExternal', 'bool', [])
klass.add_method('tileWidth', 'int', [])
klass.add_method('tileHeight', 'int', [])
klass.add_method('tileSpacing', 'int', [])
klass.add_method('margin', 'int', [])
#klass.add_method('tileOffset', 'QPoint', [])
#klass.add_method('loadFromImage', 'bool', [('QString','file')], is_pure_virtual=True)
klass.add_method('tileAt', retval('Tiled::Tile*',caller_owns_return=False), [('int','id')])
klass.add_method('tileCount', 'int', [])
klass.add_method('columnCount', 'int', [])
klass.add_method('imageWidth', 'int', [])
klass.add_method('imageHeight', 'int', [])

cls_tile.add_method('tileset', retval('Tiled::Tileset*',caller_owns_return=False), [])

cls_layer = tiled.add_class('Layer')

#mod.add_container('QList<Tileset>', retval('Tileset'), 'list')

klass = tiled.add_class('Map')
klass.add_enum('Orientation', ('Unknown','Orthogonal','Isometric'))
klass.add_copy_constructor()
klass.add_constructor([param('Orientation','orient'), param('int','w'), param('int','h'), param('int','tileW'), param('int','tileH')])
klass.add_method('setOrientation', None, [param('Orientation','o')])
klass.add_method('width', 'int', [])
klass.add_method('setWidth', None, [('int','w')])
klass.add_method('height', 'int', [])
klass.add_method('setHeight', None, [('int','h')])
klass.add_method('tileWidth', 'int', [])
klass.add_method('tileHeight', 'int', [])
klass.add_method('layerCount', 'int', [])
klass.add_method('tileLayerCount', 'int', [])
klass.add_method('objectGroupCount', 'int', [])
klass.add_method('addTileset', None, [param('Tileset*','ts',transfer_ownership=True)])
klass.add_method('insertTileset', None, [('int','pos'),param('Tileset*','ts',transfer_ownership=True)])
klass.add_method('indexOfTileset', 'int', [param('Tileset*','ts',transfer_ownership=True)])
klass.add_method('removeTilesetAt', None, [('int','pos')])
klass.add_method('replaceTileset', None, [param('Tileset*','oldts',transfer_ownership=True),param('Tileset*','newts',transfer_ownership=True)])
#klass.add_method('tilesets', 'std::list<Tileset>', [])
#klass.add_method('tilesets', retval('std::list<Tileset>'), [])
klass.add_method('isTilesetUsed', 'bool', [param('Tileset*','ts',transfer_ownership=True)])
cls_map = klass

klass = tiled.add_class('Cell')
klass.add_constructor([param('Tiled::Tile*','tile',transfer_ownership=True)]) # ok

klass = tiled.add_class('TileLayer', cls_layer)
klass.add_constructor([param('QString','name'), param('int','x'), param('int','y'), param('int','w'), param('int','h')])
klass.add_method('cellAt', retval('Tiled::Cell'), [('int','x'),('int','y')])
klass.add_method('setCell', None, [('int','x'),('int','y'),('Cell','c')])
klass.add_method('referencesTileset', 'bool', [param('Tileset*','ts',transfer_ownership=False)])
klass.add_method('isEmpty', 'bool', [])

cls_map.add_method('addLayer', None, [param('TileLayer*','l',transfer_ownership=True)])
cls_map.add_method('layerAt', retval('Tiled::Layer*',caller_owns_return=False,reference_existing_object=True), [('int','idx')])

klass = mod.add_class('QPointF')
klass.add_method('x', 'int', [])
klass.add_method('setX', None, [('int','x')])
klass.add_method('y', 'int', [])
klass.add_method('setY', None, [('int','y')])

klass = mod.add_class('QSizeF')
klass.add_method('width', 'int', [])
klass.add_method('setWidth', None, [('int','w')])
klass.add_method('height', 'int', [])
klass.add_method('setHeight', None, [('int','h')])

klass = tiled.add_class('Object')
klass.add_method('property', 'QString', [('QString','prop')])
klass.add_method('setProperty', None, [('QString','prop'),('QString','val')])

klass = tiled.add_class('MapObject', klass)
klass.add_constructor([])
klass.add_constructor([param('QString','name'), param('QString','type'), param('QPointF','pos'), param('QSizeF','size') ])
klass.add_enum('Shape', ('Rectangle','Polygon','Polyline'))
klass.add_method('setPosition', None, [param('QPointF','pos')])
klass.add_method('x', 'int', [])
klass.add_method('setX', None, [('int','x')])
klass.add_method('y', 'int', [])
klass.add_method('setY', None, [('int','y')])
klass.add_method('setSize', None, [param('QSizeF','size')])
klass.add_method('width', 'int', [])
klass.add_method('setWidth', None, [('int','w')])
klass.add_method('height', 'int', [])
klass.add_method('setHeight', None, [('int','h')])
#klass.add_method('setPolygon', None, [param('QPolygonF','pol')])
#klass.add_method('polygon', 'QPolygonF', [])
klass.add_method('setShape', None, [param('Shape','s')])
klass.add_method('shape', 'Shape', [])
#klass.add_method('bounds', 'QRectF', [])
klass.add_method('setTile', None, [param('Tiled::Tile*','t',transfer_ownership=False)])
klass.add_method('tile', retval('Tiled::Tile*',caller_owns_return=False), [])
#klass.add_method('setObjectGroup', 'ObjectGroup*', [])
#klass.add_method('objectGroup', 'ObjectGroup*', [])

klass = tiled.add_class('ObjectGroup', cls_layer)
klass.add_constructor([param('QString','name'), param('int','x'), param('int','y'), param('int','w'), param('int','h')])
klass.add_method('addObject', None, [param('MapObject*','mo',transfer_ownership=False)])
klass.add_method('insertObject', None, [('int','idx'),param('MapObject*','mo',transfer_ownership=False)])
klass.add_method('removeObject', 'int', [param('MapObject*','mo',transfer_ownership=False)])
#klass.add_method('objectsBoundingRect', 'QRectF', [])
#klass.add_method('usedTilesets', 'QSet<Tileset*>', [])
klass.add_method('referencesTileset', 'bool', [param('Tileset*','ts',transfer_ownership=False)])

cls_layer.add_method('name', 'QString', [])
cls_layer.add_method('setName', None, [('QString','name')])
cls_layer.add_method('opacity', 'float', [])
cls_layer.add_method('setOpacity', None, [('float','opacity')])
cls_layer.add_method('isVisible', 'bool', [])
cls_layer.add_method('setVisible', None, [('bool','visible')])
cls_layer.add_method('map', retval('Tiled::Map*',caller_owns_return=False), [])
cls_layer.add_method('setMap', None, [param('Tiled::Map*','map',transfer_ownership=False)])
cls_layer.add_method('x', 'int', [])
cls_layer.add_method('setX', None, [('int','x')])
cls_layer.add_method('y', 'int', [])
cls_layer.add_method('setY', None, [('int','y')])
cls_layer.add_method('setPosition', None, [('int','x'),('int','y')])
cls_layer.add_method('width', 'int', [])
cls_layer.add_method('height', 'int', [])
cls_layer.add_method('asTileLayer', retval('Tiled::TileLayer*',caller_owns_return=False), [], is_virtual=True)
cls_layer.add_method('asObjectGroup', retval('Tiled::ObjectGroup*',caller_owns_return=False), [], is_virtual=True)


mod.add_function('python_register_nameFilter', 'void', [('char*','filt')], custom_name='NameFilter')

mod.add_function('loadFromImage', 'bool', [param('Tileset*','ts',transfer_ownership=False),('QString','file')])

mod.header.writeln("""Tiled::Map *python_readCb(const char *fn, void *p);""")
mod.body.writeln("""
bool loadFromImage(Tiled::Tileset *ts, QString file)
{
  QImage img(file);
  return ts->loadFromImage(img, file);
}

// how to generate this without a container?
int _wrap_convert_py2c__Tiled__Map(PyObject *value, Tiled::Map *address)
{
    PyObject *py_retval;
    PyTiledMap *tmp_Map;

    py_retval = Py_BuildValue((char *) "(O)", value);
    if (!PyArg_ParseTuple(py_retval, (char *) "O!", &PyTiledMap_Type, &tmp_Map)) {
        Py_DECREF(py_retval);
        return 0;
    }
    *address = *tmp_Map->obj;
    Py_DECREF(py_retval);
    return 1;
}

Tiled::Map *python_readCb(const char *fn, void *p) {
  PyObject *callback = (PyObject*) p;
  PyObject *ret = PyObject_CallFunction(callback, (char*) "s", fn);
  Tiled::Map *m = new Tiled::Map(Tiled::Map::Orthogonal, 10,10, 16,16);
  _wrap_convert_py2c__Tiled__Map(ret, m);
  Py_DECREF(ret);
  return m;
}""")
mod.add_function("python_register_read_cb", None, [Parameter.new("PyCallback", "cb")], custom_name='Read')

mod.header.writeln("""bool python_writeCb(const Tiled::Map *map, const char *fn, void *p);""")
mod.body.writeln("""bool python_writeCb(const Tiled::Map *map, const char *fn, void *p) {
  PyObject *callback = (PyObject*) p;
  Tiled::Map *ncmap = map->clone(); // shouldn't have to do this, figure something out
  PyTiledMap *py_TiledMap;
  py_TiledMap = PyObject_New(PyTiledMap, &PyTiledMap_Type);
  py_TiledMap->flags = PYBINDGEN_WRAPPER_FLAG_NONE;
  py_TiledMap->obj = ncmap;
  PyObject *result = PyObject_CallFunction(callback, (char*) "Os", py_TiledMap, fn);
  if (result == NULL) return false;
  bool ret = PyObject_IsTrue(result);
  Py_DECREF(result);
  return ret;
}""")
mod.add_function("python_register_write_cb", None,  [Parameter.new("PyCallback", "cb")], custom_name='Write')

mod.header.writeln("""bool python_supportsCb(const char *fn, void *p);""")
mod.body.writeln("""bool python_supportsCb(const char *fn, void *p) {
  PyObject *callback = (PyObject*) p;
  PyObject *result = PyObject_CallFunction(callback, (char*) "s", fn);
  if (result == NULL) return false;
  bool ret = PyObject_IsTrue(result);
  Py_DECREF(result);
  return ret;
}""")
mod.add_function("python_register_supports_cb", None,  [Parameter.new("PyCallback", "cb")], custom_name='Supports')


with open('pythonbind.cpp','w') as fh:
  mod.generate(fh)

