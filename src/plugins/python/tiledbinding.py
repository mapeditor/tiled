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

  def __init__(self, ctype, name, direction=Parameter.DIRECTION_IN, is_const=False,
               default_value=None, callback=None):
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

class QStringReturnValue(ReturnValue):
  CTYPES = ['QString']

  def get_c_error_return(self):
    return "return QString();"

  def convert_python_to_c(self, wrapper):
    #raise NotImplementedError # TODO (needed only for virtual methods where C calls Python code)
    ptr = wrapper.declarations.declare_variable("const char *", "retval_ptr")
    len_ = wrapper.declarations.declare_variable("Py_ssize_t", "retval_len")
    wrapper.parse_params.add_parameter("s#", ['&'+ptr, '&'+len_])
    wrapper.after_call.write_code("%s = QString(%s);" % (self.value, ptr))

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

## QT classes, could just as well use PyQT but so far this is such a small subset..
cls_qpointf = mod.add_class('QPointF')
cls_qpointf.add_method('x', 'int', [])
cls_qpointf.add_method('setX', None, [('int','x')])
cls_qpointf.add_method('y', 'int', [])
cls_qpointf.add_method('setY', None, [('int','y')])

cls_sizef = mod.add_class('QSizeF')
cls_sizef.add_method('width', 'int', [])
cls_sizef.add_method('setWidth', None, [('int','w')])
cls_sizef.add_method('height', 'int', [])
cls_sizef.add_method('setHeight', None, [('int','h')])

cls_qrgb = mod.add_class('QRgb')
mod.add_container('QVector<QRgb>', retval('QRgb'), 'vector')

cls_color = mod.add_class('QColor')
cls_color.add_constructor([('QRgb','col')])
cls_color.add_constructor([('int','r'), ('int','g'), ('int','b')])
cls_color.add_constructor([('int','r'), ('int','g'), ('int','b'),('int','a')])
cls_color.add_method('rgb', 'QRgb', [])
cls_color.add_method('rgba', 'QRgb', [])

cls_qimage = mod.add_class('QImage')
cls_qimage.add_enum('Format', ('Format_Invalid','Format_Mono','Format_MonoLSB',
  'Format_Indexed8','Format_RGB32','Format_ARGB32',
  'Format_ARGB32_Premultiplied','Format_RGB16',
  'Format_ARGB8565_Premultiplied','Format_RGB666',
  'Format_ARGB6666_Premultiplied','Format_RGB555',
  'Format_ARGB8555_Premultiplied','Format_RGB888','Format_RGB444',
  'Format_ARGB4444_Premultiplied'))
cls_qimage.add_constructor([])
cls_qimage.add_constructor([('int','w'), ('int','h'), ('Format','f')])
cls_qimage.add_method('color', 'QRgb', [('int','i')])
cls_qimage.add_method('colorTable', 'QVector<QRgb>', [])
cls_qimage.add_method('load', 'bool', [('const QString','fileName'),('char*','fmt')])
cls_qimage.add_method('width', 'int', [])
cls_qimage.add_method('height', 'int', [])
cls_qimage.add_method('setColor', None, [('int','i'),('QRgb','c')])
cls_qimage.add_method('setPixel', None, [('int','x'),('int','y'),
  ('unsigned int','color')])
cls_qimage.add_method('setPixel', None, [('int','x'),('int','y'),
  ('QRgb','color')])
cls_qimage.add_method('setColorTable', None, [('QVector<QRgb>','colors')])
cls_qpixmap = mod.add_class('QPixmap')
## /QT

cls_tile = tiled.add_class('Tile')
cls_tile.add_method('id', 'int', [])
#cls_tile.add_method('image', retval('QPixmap&'), [])
#cls_tile.add_method('setImage', None, [('QPixmap&','image')])
cls_tile.add_method('width', 'int', [])
cls_tile.add_method('height', 'int', [])
#cls_tile.add_method('size', 'QSize', [])

cls_tileset = tiled.add_class('Tileset')
cls_tileset.add_constructor([('QString','name'), ('int','tw'), ('int','th'),
  ('int','ts'), ('int','margin')])
cls_tileset.add_method('name', 'QString', [])
cls_tileset.add_method('setName', None, [('QString','name')])
cls_tileset.add_method('fileName', 'QString', [])
cls_tileset.add_method('setFileName', None, [('QString','name')])
cls_tileset.add_method('isExternal', 'bool', [])
cls_tileset.add_method('tileWidth', 'int', [])
cls_tileset.add_method('tileHeight', 'int', [])
cls_tileset.add_method('tileSpacing', 'int', [])
cls_tileset.add_method('margin', 'int', [])
#cls_tileset.add_method('tileOffset', 'QPoint', [])
cls_tileset.add_method('loadFromImage', 'bool',
  [('const QImage&','img'),('QString','file')])
cls_tileset.add_method('tileAt',
  retval('Tiled::Tile*',caller_owns_return=False), [('int','id')])
cls_tileset.add_method('tileCount', 'int', [])
cls_tileset.add_method('columnCount', 'int', [])
cls_tileset.add_method('imageWidth', 'int', [])
cls_tileset.add_method('imageHeight', 'int', [])
cls_tileset.add_method('setTransparentColor', None, [('QColor','col')])
cls_tileset.add_method('transparentColor', 'QColor', [])

cls_tile.add_constructor([param('const QPixmap&','image'), param('int','id'),
  param('Tileset*','ts',transfer_ownership=False)])
cls_tile.add_method('tileset',
  retval('Tiled::Tileset*',caller_owns_return=False), [])

cls_layer = tiled.add_class('Layer')

#mod.add_container('QList<Tileset>', retval('Tileset'), 'list')

mod.add_container('QList<QString>', retval('QString'), 'list')
cls_props = tiled.add_class('Properties')
cls_props.add_method('keys', 'QList<QString>', [])
#cls_propsc = tiled.add_container('QMap<QString,QString>', ('QString','QString'), 'map', cls_props)

cls_map = tiled.add_class('Map')
cls_map.add_enum('Orientation', ('Unknown','Orthogonal','Isometric'))
cls_map.add_copy_constructor()
cls_map.add_constructor([('Orientation','orient'), ('int','w'), ('int','h'),
  ('int','tileW'), ('int','tileH')])
cls_map.add_method('setOrientation', None, [('Orientation','o')])
cls_map.add_method('width', 'int', [])
cls_map.add_method('setWidth', None, [('int','w')])
cls_map.add_method('height', 'int', [])
cls_map.add_method('setHeight', None, [('int','h')])
cls_map.add_method('tileWidth', 'int', [])
cls_map.add_method('tileHeight', 'int', [])
cls_map.add_method('layerCount', 'int', [])
cls_map.add_method('tileLayerCount', 'int', [])
cls_map.add_method('objectGroupCount', 'int', [])
cls_map.add_method('addTileset', None,
  [param('Tileset*','ts',transfer_ownership=True)])
cls_map.add_method('insertTileset', None,
  [('int','pos'),param('Tileset*','ts',transfer_ownership=True)])
cls_map.add_method('indexOfTileset', 'int',
  [param('Tileset*','ts',transfer_ownership=True)])
cls_map.add_method('removeTilesetAt', None, [('int','pos')])
cls_map.add_method('replaceTileset', None,
  [param('Tileset*','oldts',transfer_ownership=True),
  param('Tileset*','newts',transfer_ownership=True)])
#cls_map.add_method('tilesets', 'std::list<Tileset>', [])
#cls_map.add_method('tilesets', retval('std::list<Tileset>'), [])
cls_map.add_method('isTilesetUsed', 'bool',
  [param('Tileset*','ts',transfer_ownership=True)])
cls_map.add_method('properties', retval('Tiled::Properties','p'), [])
cls_map.add_method('property', 'QString', [('QString','name')])
cls_map.add_method('setProperty', None, [('QString','name'),
  ('QString','value')])

cls_cell = tiled.add_class('Cell')
cls_cell.add_constructor([param('Tiled::Tile*','tile',
  transfer_ownership=True)]) # ok
cls_cell.add_instance_attribute('tile', param('Tiled::Tile*',reference_existing_object=True))

cls_tilelayer = tiled.add_class('TileLayer', cls_layer)
cls_tilelayer.add_constructor([('QString','name'), ('int','x'), ('int','y'),
  ('int','w'), ('int','h')])
cls_tilelayer.add_method('cellAt', retval('Tiled::Cell'),
  [('int','x'),('int','y')])
cls_tilelayer.add_method('setCell', None, [('int','x'),('int','y'),
  ('Cell','c')])
cls_tilelayer.add_method('referencesTileset', 'bool',
  [param('Tileset*','ts',transfer_ownership=False)])
cls_tilelayer.add_method('isEmpty', 'bool', [])

cls_map.add_method('addLayer', None,
  [param('TileLayer*','l',transfer_ownership=True)])

cls_map.add_method('layerAt',
  retval('Tiled::Layer*',caller_owns_return=False,
          reference_existing_object=True), [('int','idx')])

cls_object = tiled.add_class('Object')
cls_object.add_method('property', 'QString', [('QString','prop')])
cls_object.add_method('setProperty', None,
  [('QString','prop'),('QString','val')])

cls_mapobject = tiled.add_class('MapObject', cls_object)
cls_mapobject.add_constructor([])
cls_mapobject.add_constructor([('QString','name'), ('QString','type'),
  ('QPointF','pos'), ('QSizeF','size') ])
cls_mapobject.add_enum('Shape', ('Rectangle','Polygon','Polyline'))
cls_mapobject.add_method('setPosition', None, [('QPointF','pos')])
cls_mapobject.add_method('x', 'int', [])
cls_mapobject.add_method('setX', None, [('int','x')])
cls_mapobject.add_method('y', 'int', [])
cls_mapobject.add_method('setY', None, [('int','y')])
cls_mapobject.add_method('setSize', None, [param('QSizeF','size')])
cls_mapobject.add_method('width', 'int', [])
cls_mapobject.add_method('setWidth', None, [('int','w')])
cls_mapobject.add_method('height', 'int', [])
cls_mapobject.add_method('setHeight', None, [('int','h')])
#cls_mapobject.add_method('setPolygon', None, [param('QPolygonF','pol')])
#cls_mapobject.add_method('polygon', 'QPolygonF', [])
cls_mapobject.add_method('setShape', None, [param('Shape','s')])
cls_mapobject.add_method('shape', 'Shape', [])
#cls_mapobject.add_method('bounds', 'QRectF', [])
cls_mapobject.add_method('setTile', None,
  [param('Tiled::Tile*','t',transfer_ownership=False)])
cls_mapobject.add_method('tile',
  retval('Tiled::Tile*',caller_owns_return=False), [])
#cls_mapobject.add_method('setObjectGroup', 'ObjectGroup*', [])
#cls_mapobject.add_method('objectGroup', 'ObjectGroup*', [])

cls_objectgroup = tiled.add_class('ObjectGroup', cls_layer)
cls_objectgroup.add_constructor([('QString','name'),
  ('int','x'), ('int','y'), ('int','w'), ('int','h')])
cls_objectgroup.add_method('addObject', None,
  [param('MapObject*','mo',transfer_ownership=False)])
cls_objectgroup.add_method('insertObject', None,
  [('int','idx'),param('MapObject*','mo',transfer_ownership=False)])
cls_objectgroup.add_method('removeObject', 'int',
  [param('MapObject*','mo',transfer_ownership=False)])
#cls_objectgroup.add_method('objectsBoundingRect', 'QRectF', [])
#cls_objectgroup.add_method('usedTilesets', 'QSet<Tileset*>', [])
cls_objectgroup.add_method('referencesTileset', 'bool',
  [param('Tileset*','ts',transfer_ownership=False)])

cls_layer.add_method('name', 'QString', [])
cls_layer.add_method('setName', None, [('QString','name')])
cls_layer.add_method('opacity', 'float', [])
cls_layer.add_method('setOpacity', None, [('float','opacity')])
cls_layer.add_method('isVisible', 'bool', [])
cls_layer.add_method('setVisible', None, [('bool','visible')])
cls_layer.add_method('map', retval('Tiled::Map*',caller_owns_return=False), [])
cls_layer.add_method('setMap', None,
  [param('Tiled::Map*','map',transfer_ownership=False)])
cls_layer.add_method('x', 'int', [])
cls_layer.add_method('setX', None, [('int','x')])
cls_layer.add_method('y', 'int', [])
cls_layer.add_method('setY', None, [('int','y')])
cls_layer.add_method('setPosition', None, [('int','x'),('int','y')])
cls_layer.add_method('width', 'int', [])
cls_layer.add_method('height', 'int', [])
cls_layer.add_method('asTileLayer',
  retval('Tiled::TileLayer*',caller_owns_return=False), [], is_virtual=True)
cls_layer.add_method('asObjectGroup',
  retval('Tiled::ObjectGroup*',caller_owns_return=False), [], is_virtual=True)


mod.body.writeln("""
bool isTileLayerAt(Tiled::Map *map, int idx) {
  return (dynamic_cast<const Tiled::TileLayer*>(map->layerAt(idx)) != 0);
}
bool isObjectGroupAt(Tiled::Map *map, int idx) {
  return (dynamic_cast<const Tiled::ObjectGroup*>(map->layerAt(idx)) != 0);
}
Tiled::TileLayer* tileLayerAt(Tiled::Map *map, int idx) {
  return static_cast<Tiled::TileLayer*>(map->layerAt(idx));
}
Tiled::ObjectGroup* objectGroupAt(Tiled::Map *map, int idx) {
  return static_cast<Tiled::ObjectGroup*>(map->layerAt(idx));
}
""")

mod.add_function('isTileLayerAt', 'bool',
  [param('Tiled::Map*','map',transfer_ownership=False),('int','idx')])
mod.add_function('isObjectGroupAt', 'bool',
  [param('Tiled::Map*','map',transfer_ownership=False),('int','idx')])

mod.add_function('tileLayerAt',
  retval('Tiled::TileLayer*',caller_owns_return=False,
          reference_existing_object=True),
  [param('Tiled::Map*','map',transfer_ownership=False),('int','idx')])
mod.add_function('objectGroupAt',
  retval('Tiled::ObjectGroup*',caller_owns_return=False,
          reference_existing_object=True),
  [param('Tiled::Map*','map',transfer_ownership=False),('int','idx')])



mod.add_function('loadTilesetFromFile', 'bool',
  [param('Tileset*','ts',transfer_ownership=False),('QString','file')])

mod.body.writeln("""
bool loadTilesetFromFile(Tiled::Tileset *ts, QString file)
{
  QImage img(file);
  return ts->loadFromImage(img, file);
}
""")

cls_pp = mod.add_class('PythonScript',
  allow_subclassing=True,
  foreign_cpp_namespace='Python',
  custom_name='Plugin')
#cls_pp.add_method('nameFilter', 'QString', [])
#cls_pp.add_method('supportsFile', 'bool', [('const QString','fileName')])
#cls_pp.add_method('read', retval('Tiled::Map'), [('const QString','fileName')])
#cls_pp.add_method('write', 'bool',
#  [param('const Tiled::Map*','map',transfer_ownership=False),
#  ('const QString','fileName')])

with open('pythonbind.cpp','w') as fh:
  mod.generate(fh)

  import pybindgen.typehandlers.codesink as cs
  sink = cs.MemoryCodeSink()
  mod.generate_python_to_c_type_converter(
    utils.eval_retval("Tiled::Map"),
    sink)
  mod.generate_c_to_python_type_converter(
    utils.eval_retval("const Tiled::Map"),
    sink)
  print >>fh, sink.flush()

