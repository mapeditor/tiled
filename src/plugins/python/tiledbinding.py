"""
 Python Tiled Plugin
 Copyright 2012-2013, Samuli Tuomola <samuli@tuomola.net>

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

mod = Module('tiled')
mod.add_include('"pythonplugin.h"')
mod.add_include('"map.h"')
mod.add_include('"layer.h"')
mod.add_include('"tile.h"')
mod.add_include('"mapobject.h"')
mod.add_include('"imagelayer.h"')
mod.add_include('"tilelayer.h"')
mod.add_include('"objectgroup.h"')
mod.add_include('"tileset.h"')

mod.header.writeln('#pragma GCC diagnostic ignored "-Wmissing-field-initializers"')

# one day PyQt/PySide could be considered
import qtbinding
qtbinding.generate(mod)

#mod.add_include('"qtbind.h"')

tiled = mod.add_cpp_namespace('Tiled')

cls_props = tiled.add_class('Properties')
cls_props.add_method('keys', 'QList<QString>', [])
#cls_propsc = tiled.add_container('QMap<QString,QString>', ('QString','QString'), 'map', cls_props)

cls_object = tiled.add_class('Object')
cls_object.add_method('properties', retval('Tiled::Properties','p'), [])
cls_object.add_method('propertyAsString', 'QString', [('QString','prop')])
cls_object.add_method('setProperty', None,
    [('QString','prop'),('QString','val')])

cls_tile = tiled.add_class('Tile', cls_object)
cls_tile.add_method('id', 'int', [])
cls_tile.add_method('image', retval('const QPixmap&'), [])
cls_tile.add_method('setImage', None, [('const QPixmap&','image')])
cls_tile.add_method('width', 'int', [])
cls_tile.add_method('height', 'int', [])
#cls_tile.add_method('size', 'QSize', [])

cls_tileset = tiled.add_class('Tileset', cls_object)
cls_sharedtileset = tiled.add_class('SharedTileset')
cls_sharedtileset.add_method('data', retval('Tiled::Tileset*',reference_existing_object=True), [])

cls_tileset.add_method('create', 'Tiled::SharedTileset',
                       [('QString','name'), ('int','tileWidth'), ('int','tileHeight'), ('int','tileSpacing'), ('int','margin')],
                       is_static=True)
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
    retval('Tiled::Tile*',reference_existing_object=True), [('int','id')])
cls_tileset.add_method('tileCount', 'int', [])
cls_tileset.add_method('columnCount', 'int', [])
cls_tileset.add_method('imageWidth', 'int', [])
cls_tileset.add_method('imageHeight', 'int', [])
cls_tileset.add_method('setTransparentColor', None, [('QColor','col')])
cls_tileset.add_method('transparentColor', 'QColor', [])

cls_tile.add_constructor([param('const QPixmap&','image'), param('int','id'),
    param('Tileset*','tileset',transfer_ownership=False)])
cls_tile.add_method('tileset',
    retval('Tiled::Tileset*',reference_existing_object=True), [])

cls_layer = tiled.add_class('Layer', cls_object)

#mod.add_container('QList<Tiled::Tileset*>',
#                retval('Tiled::Tileset*',caller_owns_return=False), 'list')

cls_map = tiled.add_class('Map', cls_object)
cls_map.add_enum('Orientation', ('Unknown','Orthogonal','Isometric'))
cls_map.add_copy_constructor()
cls_map.add_constructor([('Orientation','orient'), ('int','w'), ('int','h'),
    ('int','tileW'), ('int','tileH')])
cls_map.add_method('orientation', 'Orientation', [])
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
    [param('SharedTileset','tileset')])
cls_map.add_method('insertTileset', None,
    [('int','pos'),param('SharedTileset','tileset')])
cls_map.add_method('indexOfTileset', 'int',
    [param('const SharedTileset &','tileset')])
cls_map.add_method('removeTilesetAt', None, [('int','pos')])
cls_map.add_method('replaceTileset', None,
    [param('SharedTileset','oldTileset'),
     param('SharedTileset','newTileset')])
cls_map.add_method('tilesetAt',
    retval('Tiled::SharedTileset'),
    [('int','idx')])
cls_map.add_method('tilesetCount', 'int', [])
cls_map.add_method('isTilesetUsed', 'bool',
    [param('const Tileset*','tileset')])

cls_cell = tiled.add_class('Cell')
cls_cell.add_constructor([param('Tiled::Tile*','tile',
    transfer_ownership=False)])
cls_cell.add_method('isEmpty', 'bool', [])
cls_cell.add_method('tile', retval('Tiled::Tile*',reference_existing_object=True), [])

cls_tilelayer = tiled.add_class('TileLayer', cls_layer)
cls_tilelayer.add_constructor([('QString','name'), ('int','x'), ('int','y'),
    ('int','w'), ('int','h')])
cls_tilelayer.add_method('width', 'int', [])
cls_tilelayer.add_method('height', 'int', [])
cls_tilelayer.add_method('cellAt', retval('Tiled::Cell'),
    [('int','x'),('int','y')])
cls_tilelayer.add_method('setCell', None, [('int','x'),('int','y'),
    ('Cell','c')])
cls_tilelayer.add_method('referencesTileset', 'bool',
    [param('Tileset*','ts',transfer_ownership=False)])
cls_tilelayer.add_method('isEmpty', 'bool', [])

cls_imagelayer = tiled.add_class('ImageLayer', cls_layer)
cls_imagelayer.add_constructor([('QString','name'), ('int','x'), ('int','y')])
cls_imagelayer.add_method('loadFromImage', 'bool',
    [('const QImage&','img'),('QString','file')])
cls_imagelayer.add_method('image', retval('const QPixmap&'), [])
cls_imagelayer.add_method('setImage', None, [('const QPixmap&','image')])

cls_mapobject = tiled.add_class('MapObject', cls_object)
cls_mapobject.add_constructor([])
cls_mapobject.add_constructor([('QString','name'), ('QString','type'),
    ('QPointF','pos'), ('QSizeF','size') ])
cls_mapobject.add_enum('Shape', ('Rectangle','Polygon','Polyline'))
cls_mapobject.add_method('setPosition', None, [('QPointF','pos')])
cls_mapobject.add_method('x', 'double', [])
cls_mapobject.add_method('setX', None, [('double','x')])
cls_mapobject.add_method('y', 'double', [])
cls_mapobject.add_method('setY', None, [('double','y')])
cls_mapobject.add_method('setSize', None, [param('QSizeF','size')])
cls_mapobject.add_method('width', 'double', [])
cls_mapobject.add_method('setWidth', None, [('double','w')])
cls_mapobject.add_method('height', 'double', [])
cls_mapobject.add_method('setHeight', None, [('double','h')])
#cls_mapobject.add_method('setPolygon', None, [param('QPolygonF','pol')])
#cls_mapobject.add_method('polygon', 'QPolygonF', [])
cls_mapobject.add_method('setShape', None, [param('Shape','s')])
cls_mapobject.add_method('shape', 'Shape', [])
#cls_mapobject.add_method('bounds', 'QRectF', [])
cls_mapobject.add_method('setCell', None, [param('const Tiled::Cell','c',)])
cls_mapobject.add_method('cell', retval('const Tiled::Cell'), [])
#cls_mapobject.add_method('setObjectGroup', 'ObjectGroup*', [])
#cls_mapobject.add_method('objectGroup', 'ObjectGroup*', [])
cls_mapobject.add_method('rotation', 'double', [])
cls_mapobject.add_method('setRotation', None, [('double','r')])
cls_mapobject.add_method('isVisible', 'bool', [])
cls_mapobject.add_method('setVisible', None, [('bool','v')])
cls_mapobject.add_method('name', 'QString', [])
cls_mapobject.add_method('setName', None, [('QString','n')])
cls_mapobject.add_method('type', 'QString', [])
cls_mapobject.add_method('setType', None, [('QString','n')])

cls_objectgroup = tiled.add_class('ObjectGroup', cls_layer)
cls_objectgroup.add_constructor([('QString','name'), ('int','x'), ('int','y')])
cls_objectgroup.add_method('addObject', None,
    [param('MapObject*','object',transfer_ownership=True)])
cls_objectgroup.add_method('insertObject', None,
    [('int','idx'),param('MapObject*','object',transfer_ownership=False)])
cls_objectgroup.add_method('removeObject', 'int',
    [param('MapObject*','object',transfer_ownership=False)])
#cls_tilelayer.add_method('cellAt', retval('Tiled::Cell'),
#    [('int','x'),('int','y')])
cls_objectgroup.add_method('objectAt', retval('Tiled::MapObject*',reference_existing_object=True),[('int','index')])
cls_objectgroup.add_method('objectCount', 'int',[])
#cls_objectgroup.add_method('objectsBoundingRect', 'QRectF', [])
#cls_objectgroup.add_method('usedTilesets', 'QSet<Tileset*>', [])
cls_objectgroup.add_method('referencesTileset', 'bool',
    [param('Tileset*','ts',transfer_ownership=False)])
#MapObject *objectAt(int index)

cls_map.add_method('addLayer', None,
    [param('ImageLayer*','l',transfer_ownership=True)])
cls_map.add_method('addLayer', None,
    [param('TileLayer*','l',transfer_ownership=True)])
cls_map.add_method('addLayer', None,
    [param('ObjectGroup*','l',transfer_ownership=True)])

cls_map.add_method('layerAt',
    retval('Tiled::Layer*',reference_existing_object=True), [('int','idx')])

cls_layer.add_method('name', 'QString', [])
cls_layer.add_method('setName', None, [('QString','name')])
cls_layer.add_method('opacity', 'double', [])
cls_layer.add_method('setOpacity', None, [('double','opacity')])
cls_layer.add_method('isVisible', 'bool', [])
cls_layer.add_method('setVisible', None, [('bool','visible')])
cls_layer.add_method('map', retval('Tiled::Map*',reference_existing_object=True), [])
cls_layer.add_method('x', 'int', [])
cls_layer.add_method('setX', None, [('int','x')])
cls_layer.add_method('y', 'int', [])
cls_layer.add_method('setY', None, [('int','y')])
cls_layer.add_method('setPosition', None, [('int','x'),('int','y')])
cls_layer.add_method('asTileLayer',
    retval('Tiled::TileLayer*',reference_existing_object=True), [], is_virtual=True)
cls_layer.add_method('asObjectGroup',
    retval('Tiled::ObjectGroup*',reference_existing_object=True), [], is_virtual=True)


mod.body.writeln("""
bool isImageLayerAt(Tiled::Map *map, int idx) {
    return (dynamic_cast<const Tiled::ImageLayer*>(map->layerAt(idx)) != 0);
}
bool isTileLayerAt(Tiled::Map *map, int idx) {
    return (dynamic_cast<const Tiled::TileLayer*>(map->layerAt(idx)) != 0);
}
bool isObjectGroupAt(Tiled::Map *map, int idx) {
    return (dynamic_cast<const Tiled::ObjectGroup*>(map->layerAt(idx)) != 0);
}
Tiled::ImageLayer* imageLayerAt(Tiled::Map *map, int idx) {
    return static_cast<Tiled::ImageLayer*>(map->layerAt(idx));
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
    retval('Tiled::TileLayer*',reference_existing_object=True),
    [param('Tiled::Map*','map',transfer_ownership=False),('int','idx')])
mod.add_function('objectGroupAt',
    retval('Tiled::ObjectGroup*',reference_existing_object=True),
    [param('Tiled::Map*','map',transfer_ownership=False),('int','idx')])



mod.add_function('loadTilesetFromFile', 'bool',
    [param('Tileset*','ts',transfer_ownership=False),('QString','file')])

mod.body.writeln("""
static bool loadTilesetFromFile(Tiled::Tileset *ts, const QString &file)
{
    QImage img(file);
    return ts->loadFromImage(img, file);
}
""")

"""
 C++ class PythonScript is seen as Tiled.Plugin from Python script
 (naming describes the opposite side from either perspective)
"""
cls_pp = mod.add_class('PythonScript',
    allow_subclassing=True,
    foreign_cpp_namespace='Python',
    custom_name='Plugin')

"""
 PythonPlugin implements LoggingInterface for messaging to Tiled
"""
cls_logi = tiled.add_class('LoggingInterface', destructor_visibility='private')
cls_logi.add_enum('OutputType', ('INFO','ERROR'))
cls_logi.add_method('log', 'void', [('OutputType','type'),('const QString','msg')],
    is_virtual=True)


with open('pythonbind.cpp','w') as fh:
    import pybindgen.typehandlers.codesink as cs
    sink = cs.MemoryCodeSink()

    print >>fh, """
#ifdef __MINGW32__
#include <cmath> // included before Python.h to fix ::hypot not declared issue
#endif
"""

    mod.generate(fh)

    print >>fh, """
PyObject* _wrap_convert_c2py__Tiled__LoggingInterface(Tiled::LoggingInterface *cvalue)
{
        PyObject *py_retval;
        PyTiledLoggingInterface *py_LoggingInterface;

        py_LoggingInterface = PyObject_New(PyTiledLoggingInterface, &PyTiledLoggingInterface_Type);
        py_LoggingInterface->flags = PYBINDGEN_WRAPPER_FLAG_NONE;
        py_LoggingInterface->obj = cvalue;
        py_retval = Py_BuildValue((char *) "N", py_LoggingInterface);
        return py_retval;
}
"""
    #mod.generate_c_to_python_type_converter(
    #  utils.eval_retval(retval("Tiled::LoggingInterface")),
    #  sink)
    mod.generate_python_to_c_type_converter(
        utils.eval_retval(retval('Tiled::Map*',caller_owns_return=True)),
        sink)
    mod.generate_c_to_python_type_converter(
        utils.eval_retval("const Tiled::Map"),
        sink)

    print >>fh, sink.flush()

# vim: ai ts=4 sts=4 et sw=4 ft=python
