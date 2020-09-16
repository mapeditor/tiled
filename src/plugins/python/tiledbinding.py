#!/usr/bin/env python3
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

from __future__ import print_function
from functools import wraps
from operator import attrgetter, itemgetter
from pybindgen import *
from collections import OrderedDict


class SimpleSortedDict(OrderedDict):
    "naive and ineffecient but adequate for this use"
    def items(self):
        return sorted([[k, self[k]] for k in self], key=itemgetter(0))


def patch_a_prop(func, prop, value_factory):
    """replace an object property after it's given method has executed
    """
    assert callable(value_factory)
    def _decorate(obj, *args, **kwargs):
        ret = func(obj, *args, **kwargs)
        setattr(obj, prop, value_factory())
        return ret

    return wraps(func)(_decorate)


# after a new pybindgen container is instantiated, replace it's methods dictionary
Module.__init__ = patch_a_prop(Module.__init__, 'methods', lambda:SimpleSortedDict())
CppClass.__init__ = patch_a_prop(CppClass.__init__, 'methods', lambda:SimpleSortedDict())


mod = Module('tiled')
mod.functions = SimpleSortedDict()

mod.add_include('"pythonplugin.h"')
mod.add_include('"grouplayer.h"')
mod.add_include('"imagelayer.h"')
mod.add_include('"layer.h"')
mod.add_include('"logginginterface.h"')
mod.add_include('"map.h"')
mod.add_include('"mapobject.h"')
mod.add_include('"objectgroup.h"')
mod.add_include('"tile.h"')
mod.add_include('"tilelayer.h"')
mod.add_include('"tileset.h"')
mod.add_include('"tilesetmanager.h"')

mod.header.writeln('#ifndef _MSC_VER')
mod.header.writeln('#pragma GCC diagnostic ignored "-Wmissing-field-initializers"')
mod.header.writeln('#endif')

# one day PyQt/PySide could be considered
import qtbinding
qtbinding.generate(mod)

#mod.add_include('"qtbind.h"')

tiled = mod.add_cpp_namespace('Tiled')

cls_properties = tiled.add_class('Properties')
cls_properties.add_copy_constructor()
cls_properties.add_method('keys', 'QList<QString>', [])
#cls_propsc = tiled.add_container('QMap<QString,QString>', ('QString','QString'), 'map', cls_properties)

cls_object = tiled.add_class('Object')
cls_object.add_method('properties', retval('Tiled::Properties','p'), [])
cls_object.add_method('propertyAsString', 'QString', [('QString','prop')])
cls_object.add_method('setProperty', None,
    [('QString','prop'),('QString','val')])
cls_object.add_method('setProperty', None,
    [('QString','prop'),('int','val')])
cls_object.add_method('setProperty', None,
    [('QString','prop'),('bool','val')])
cls_object.add_method('propertyType', 'QString', [('QString','prop')])

cls_tile = tiled.add_class('Tile', cls_object)
cls_tile.add_method('id', 'int', [])
cls_tile.add_method('image', retval('const QPixmap&'), [])
cls_tile.add_method('setImage', None, [('const QPixmap&','image')])
cls_tile.add_method('width', 'int', [])
cls_tile.add_method('height', 'int', [])
cls_tile.add_method('size', 'QSize', [])
cls_tile.add_method('type', 'QString', [])

cls_tileset = tiled.add_class('Tileset', cls_object)
cls_sharedtileset = tiled.add_class('SharedTileset')
cls_sharedtileset.add_copy_constructor()
cls_sharedtileset.add_method('data', retval('Tiled::Tileset*',reference_existing_object=True), [])

#cls_tileset.add_enum('Orientation', ('Orthogonal','Isometric'))
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
cls_tileset.add_method('tileOffset', 'QPoint', [])
cls_tileset.add_method('setTileOffset', None, [('QPoint','offset')])
#cls_tileset.add_method('orientation', 'Orientation', [])
#cls_tileset.add_method('setOrientation', None, [('Orientation', 'orientation')])
cls_tileset.add_method('gridSize', 'QSize', [])
cls_tileset.add_method('setGridSize', None, [('QSize', 'gridSize')])
cls_tileset.add_method('loadFromImage', 'bool',
    [('const QImage&','img'),('QString','file')])
cls_tileset.add_method('loadImage', 'bool', [])
cls_tileset.add_method('findTile',
    retval('Tiled::Tile*',reference_existing_object=True), [('int','id')])
cls_tileset.add_method('tileAt',
    retval('Tiled::Tile*',reference_existing_object=True), [('int','id')])
cls_tileset.add_method('tileCount', 'int', [])
cls_tileset.add_method('columnCount', 'int', [])
cls_tileset.add_method('rowCount', 'int', [])
cls_tileset.add_method('imageWidth', 'int', [])
cls_tileset.add_method('imageHeight', 'int', [])
cls_tileset.add_method('setTransparentColor', None, [('QColor','col')])
cls_tileset.add_method('transparentColor', 'QColor', [])
cls_tileset.add_method('imageSourceString', 'QString', [])
cls_tileset.add_method('setImageSource', None, [('QString','source')])
cls_tileset.add_method('isCollection', 'bool', [])
cls_tileset.add_method('sharedPointer', 'Tiled::SharedTileset', [])

cls_tile.add_constructor([param('const QPixmap&','image'), param('int','id'),
    param('Tileset*','tileset',transfer_ownership=False)])
cls_tile.add_method('tileset',
    retval('Tiled::Tileset*',reference_existing_object=True), [])

cls_layer = tiled.add_class('Layer', cls_object)

#mod.add_container('QList<Tiled::Tileset*>',
#                retval('Tiled::Tileset*',caller_owns_return=False), 'list')

cls_map = tiled.add_class('Map', cls_object)
cls_map.add_enum('Orientation', ('Unknown','Orthogonal','Isometric','Staggered','Hexagonal'))
cls_map.add_enum('LayerDataFormat', ('XML','Base64','Base64Gzip','Base64Zlib','CSV'))
cls_map.add_enum('RenderOrder', ('RightDown','RightUp','LeftDown','LeftUp'))
cls_map.add_enum('StaggerAxis', ('StaggerX','StaggerY'))
cls_map.add_enum('StaggerIndex', ('StaggerOdd','StaggerEven'))
cls_map.add_constructor([('Orientation','orient'), ('int','w'), ('int','h'),
    ('int','tileW'), ('int','tileH')])
cls_map.add_method('orientation', 'Orientation', [])
cls_map.add_method('setOrientation', None, [('Orientation','orientation')])
cls_map.add_method('renderOrder', 'RenderOrder', [])
cls_map.add_method('setRenderOrder', None, [('RenderOrder','renderOrder')])
cls_map.add_method('width', 'int', [])
cls_map.add_method('setWidth', None, [('int','w')])
cls_map.add_method('height', 'int', [])
cls_map.add_method('setHeight', None, [('int','h')])
cls_map.add_method('tileWidth', 'int', [])
cls_map.add_method('tileHeight', 'int', [])
cls_map.add_method('tileSize', 'QSize', [])
cls_map.add_method('infinite', 'bool', [])
cls_map.add_method('setInfinite', None, [('bool','infinite')])
cls_map.add_method('hexSideLength', 'int', [])
cls_map.add_method('setHexSideLength', None, [('int', 'hexSideLength')])
cls_map.add_method('staggerAxis', 'StaggerAxis', [])
cls_map.add_method('setStaggerAxis', None, [('StaggerAxis','staggerAxis')])
cls_map.add_method('staggerIndex', 'StaggerIndex', [])
cls_map.add_method('setStaggerIndex', None, [('StaggerIndex','staggerIndex')])
cls_map.add_method('layerCount', 'int', [])
cls_map.add_method('tileLayerCount', 'int', [])
cls_map.add_method('objectGroupCount', 'int', [])
cls_map.add_method('imageLayerCount', 'int', [])
cls_map.add_method('groupLayerCount', 'int', [])
cls_map.add_method('backgroundColor', 'QColor', [])
cls_map.add_method('setBackgroundColor', None, [('QColor','col')])
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
    [('int','index')])
cls_map.add_method('tilesetCount', 'int', [])
cls_map.add_method('isTilesetUsed', 'bool',
    [param('const Tileset*','tileset')])
cls_map.add_method('nextLayerId', 'int', [])
cls_map.add_method('setNextLayerId', None, [('int', 'nextLayerId')])
cls_map.add_method('nextObjectId', 'int', [])
cls_map.add_method('setNextObjectId', None, [('int', 'nextObjectId')])

cls_cell = tiled.add_class('Cell')
cls_cell.add_constructor([param('Tiled::Tile*','tile',transfer_ownership=False)])
cls_cell.add_copy_constructor()
cls_cell.add_binary_comparison_operator('==')
cls_cell.add_binary_comparison_operator('!=')
cls_cell.add_method('isEmpty', 'bool', [])
cls_cell.add_instance_attribute('flippedHorizontally', 'bool', getter='flippedHorizontally', setter='setFlippedHorizontally')
cls_cell.add_instance_attribute('flippedVertically', 'bool', getter='flippedVertically', setter='setFlippedVertically')
cls_cell.add_instance_attribute('flippedAntiDiagonally', 'bool', getter='flippedAntiDiagonally', setter='setFlippedAntiDiagonally')
cls_cell.add_instance_attribute('rotatedHexagonal120', 'bool', getter='rotatedHexagonal120', setter='setRotatedHexagonal120')
cls_cell.add_instance_attribute('checked', 'bool', getter='checked', setter='setChecked')
cls_cell.add_method('tile', retval('Tiled::Tile*',reference_existing_object=True), [])
cls_cell.add_method('tileset', retval('Tiled::Tileset*',reference_existing_object=True), [])
cls_cell.add_method('setTile', None, [param('Tiled::Tile*','tile',transfer_ownership=False)])

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

cls_grouplayer = tiled.add_class('GroupLayer', cls_layer)
cls_grouplayer.add_constructor([('QString','name'), ('int','x'), ('int','y')])
cls_grouplayer.add_method('layerCount', 'int', [])
cls_grouplayer.add_method('layerAt', retval('Tiled::Layer*',reference_existing_object=True), [('int','index')])

cls_objectgroup = tiled.add_class('ObjectGroup', cls_layer)

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
cls_mapobject.add_method('objectGroup', retval('Tiled::ObjectGroup*',reference_existing_object=True), [])
cls_mapobject.add_method('rotation', 'double', [])
cls_mapobject.add_method('setRotation', None, [('double','r')])
cls_mapobject.add_method('isVisible', 'bool', [])
cls_mapobject.add_method('setVisible', None, [('bool','v')])
cls_mapobject.add_method('name', 'QString', [])
cls_mapobject.add_method('setName', None, [('QString','n')])
cls_mapobject.add_method('type', 'QString', [])
cls_mapobject.add_method('setType', None, [('QString','n')])
cls_mapobject.add_method('effectiveType', 'QString', [])

cls_objectgroup.add_constructor([('QString','name'), ('int','x'), ('int','y')])
cls_objectgroup.add_method('addObject', None,
    [param('MapObject*','object',transfer_ownership=True)])
cls_objectgroup.add_method('insertObject', None,
    [('int','index'),param('MapObject*','object',transfer_ownership=False)])
cls_objectgroup.add_method('removeObject', 'int',
    [param('MapObject*','object',transfer_ownership=False)])
cls_objectgroup.add_method('objectAt', retval('Tiled::MapObject*',reference_existing_object=True),[('int','index')])
cls_objectgroup.add_method('objectCount', 'int',[])
#cls_objectgroup.add_method('objectsBoundingRect', 'QRectF', [])
#cls_objectgroup.add_method('usedTilesets', 'QSet<Tileset*>', [])
cls_objectgroup.add_method('referencesTileset', 'bool',
    [param('Tileset*','ts',transfer_ownership=False)])

cls_map.add_method('addLayer', None, [param('TileLayer*','l',transfer_ownership=True)])
cls_map.add_method('addLayer', None, [param('ObjectGroup*','l',transfer_ownership=True)])
cls_map.add_method('addLayer', None, [param('ImageLayer*','l',transfer_ownership=True)])
cls_map.add_method('addLayer', None, [param('GroupLayer*','l',transfer_ownership=True)])

cls_map.add_method('layerAt', retval('Tiled::Layer*',reference_existing_object=True), [('int','index')])

cls_layer.add_method('name', 'QString', [])
cls_layer.add_method('setName', None, [('QString','name')])
cls_layer.add_method('opacity', 'double', [])
cls_layer.add_method('setOpacity', None, [('double','opacity')])
cls_layer.add_method('isVisible', 'bool', [])
cls_layer.add_method('isLocked', 'bool', [])
cls_layer.add_method('isUnlocked', 'bool', [])
cls_layer.add_method('isHidden', 'bool', [])
cls_layer.add_method('setVisible', None, [('bool','visible')])
cls_layer.add_method('setLocked', None, [('bool','locked')])
cls_layer.add_method('map', retval('Tiled::Map*',reference_existing_object=True), [])
cls_layer.add_method('x', 'int', [])
cls_layer.add_method('setX', None, [('int','x')])
cls_layer.add_method('y', 'int', [])
cls_layer.add_method('setY', None, [('int','y')])
cls_layer.add_method('setPosition', None, [('int','x'),('int','y')])
cls_layer.add_method('isTileLayer', 'bool', [])
cls_layer.add_method('isObjectGroup', 'bool', [])
cls_layer.add_method('isImageLayer', 'bool', [])
cls_layer.add_method('isGroupLayer', 'bool', [])
cls_layer.add_method('asTileLayer', retval('Tiled::TileLayer*',reference_existing_object=True), [])
cls_layer.add_method('asObjectGroup', retval('Tiled::ObjectGroup*',reference_existing_object=True), [])
cls_layer.add_method('asImageLayer', retval('Tiled::ImageLayer*',reference_existing_object=True), [])
cls_layer.add_method('asGroupLayer', retval('Tiled::GroupLayer*',reference_existing_object=True), [])


mod.body.writeln("""
bool isImageLayerAt(Tiled::Map *map, int index) {
    return map->layerAt(index)->isImageLayer();
}
bool isTileLayerAt(Tiled::Map *map, int index) {
    return map->layerAt(index)->isTileLayer();
}
bool isObjectGroupAt(Tiled::Map *map, int index) {
    return map->layerAt(index)->isObjectGroup();
}
Tiled::ImageLayer* imageLayerAt(Tiled::Map *map, int index) {
    return map->layerAt(index)->asImageLayer();
}
Tiled::TileLayer* tileLayerAt(Tiled::Map *map, int index) {
    return map->layerAt(index)->asTileLayer();
}
Tiled::ObjectGroup* objectGroupAt(Tiled::Map *map, int index) {
    return map->layerAt(index)->asObjectGroup();
}
""")

mod.add_function('isImageLayerAt', 'bool',
    [param('Tiled::Map*','map',transfer_ownership=False),('int','index')])
mod.add_function('isTileLayerAt', 'bool',
    [param('Tiled::Map*','map',transfer_ownership=False),('int','index')])
mod.add_function('isObjectGroupAt', 'bool',
    [param('Tiled::Map*','map',transfer_ownership=False),('int','index')])

mod.add_function('imageLayerAt',
    retval('Tiled::ImageLayer*',reference_existing_object=True),
    [param('Tiled::Map*','map',transfer_ownership=False),('int','index')])
mod.add_function('tileLayerAt',
    retval('Tiled::TileLayer*',reference_existing_object=True),
    [param('Tiled::Map*','map',transfer_ownership=False),('int','index')])
mod.add_function('objectGroupAt',
    retval('Tiled::ObjectGroup*',reference_existing_object=True),
    [param('Tiled::Map*','map',transfer_ownership=False),('int','index')])



mod.add_function('loadTilesetFromFile', 'bool',
    [param('Tileset*','ts',transfer_ownership=False),('QString','file')])

mod.body.writeln("""
static bool loadTilesetFromFile(Tiled::Tileset *ts, const QString &file)
{
    QImage img(file);
    return ts->loadFromImage(img, file);
}
""")

mod.add_function('loadTileset', 'Tiled::SharedTileset', [('QString','file')])

mod.body.writeln("""
static Tiled::SharedTileset loadTileset(const QString &file)
{
    return Tiled::TilesetManager::instance()->loadTileset(file);
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
cls_logi.add_enum('OutputType', ('INFO','WARNING','ERROR'))
cls_logi.add_method('log', 'void', [('OutputType','type'),('const QString','msg')])


with open('pythonbind.cpp','w') as fh:
    import pybindgen.typehandlers.codesink as cs
    sink = cs.MemoryCodeSink()

    print("""
#ifdef __MINGW32__
#include <cmath> // included before Python.h to fix ::hypot not declared issue
#endif
""", file=fh)

    mod.generate(fh)

    print("""
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

int _wrap_convert_py2c__Tiled__Map___star__(PyObject *value, Tiled::Map * *address)
{
    PyObject *py_retval;
    PyTiledMap *tmp_Map;

    py_retval = Py_BuildValue((char *) "(O)", value);
    if (!PyArg_ParseTuple(py_retval, (char *) "O!", &PyTiledMap_Type, &tmp_Map)) {
        Py_DECREF(py_retval);
        return 0;
    }
    *address = tmp_Map->obj->clone().release();
    Py_DECREF(py_retval);
    return 1;
}
""", file=fh)
    #mod.generate_c_to_python_type_converter(
    #  utils.eval_retval(retval("Tiled::LoggingInterface")),
    #  sink)
    # mod.generate_python_to_c_type_converter(
    #    utils.eval_retval(retval('Tiled::Map*',caller_owns_return=True)),
    #    sink)
    mod.generate_c_to_python_type_converter(
        utils.eval_retval(retval('const Tiled::Map*',reference_existing_object=True)),
        sink)

    print(sink.flush(), file=fh)

# vim: ai ts=4 sts=4 et sw=4 ft=python
