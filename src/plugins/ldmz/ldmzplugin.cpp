/*
 * libDarnit Map File Tiled plugin
 *
 * Copyright 2012, Steven Arnow <s@rdw.se>
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

#include "ldmzplugin.h"

#include "gidmapper.h"
#include "map.h"
#include "mapobject.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "objectgroup.h"

#include <QFile>
#include <QTextStream>
#include <QtCore>
#include <QtEndian>

#include <cstdlib>
#include <cstring>

using namespace LDMZ;
using namespace Tiled;

LDMZPlugin::LDMZPlugin()
{
}

QString LDMZPlugin::nameFilter() const
{
    return tr("libDarnit Map File (*.ldmz)");
}

QString LDMZPlugin::errorString() const
{
    return mError;
}


void LDMZPlugin::writeInt(unsigned int val, FILE *fp) {
    val = qToBigEndian ((qint32)val);
    fwrite(&val, 4, 1, fp);
    
    return;
}


void LDMZPlugin::writeInts(unsigned int *val, int vals, FILE *fp) {
    int i;
    
    for (i = 0; i < vals; i++)
        writeInt(val[i], fp);
    
    return;
}


int LDMZPlugin::copyString(char *string, char *target) {
    strcpy(target, string);
    target[strlen(string)] = 0;
    return strlen(string) + 1;
}


bool LDMZPlugin::writeStrings(const Tiled::Map *map, char *strings, LDMZ_REF *ref, LDMZ_MAP *map_s, LDMZ_OBJECT *object, LDMZ_LAYER *layer_s, FILE *fp) {
    Properties::const_iterator it = map->properties().constBegin();
    Properties::const_iterator it_end = map->properties().constEnd();
    int ref_i = 0, i = 0, layers = -1, objects = 0;
    QByteArray data, zdata;
    
    map_s->main_ref = 0;
    
    for (; it != it_end; it++, ref_i++) {
        ref[ref_i].key = qToBigEndian(i);
        i += copyString(it.key().toUtf8().data(), &strings[i]);
        ref[ref_i].value = qToBigEndian(i);
        i += copyString(it.value().toUtf8().data(), &strings[i]);
    }
    
    ref[ref_i].key = ref[ref_i].value = qToBigEndian(-1);
    ref_i++;
    
    foreach (Layer *layer, map->layers()) {
        it = layer->properties().constBegin();
        it_end = layer->properties().constEnd();
        if (layer->isTileLayer()) {
            if (layer->name() != "collision") {
                layers++;
                layer_s[layers].prop_ref = qToBigEndian(ref_i);
                
                for (; it!= it_end; it++, ref_i++) {
                    ref[ref_i].key = qToBigEndian(i);
                    i += copyString(it.key().toUtf8().data(), &strings[i]);
                    ref[ref_i].value = qToBigEndian(i);
                    i += copyString(it.value().toUtf8().data(), &strings[i]);
                }
                if (!layer->name().isEmpty()) {
                    ref[ref_i].key = qToBigEndian(i);
                    i += copyString((char *) "NAME", &strings[i]);
                    ref[ref_i].value = qToBigEndian(i);
                    i += copyString(layer->name().toUtf8().data(), &strings[i]);
                    ref_i++;
                }
                ref[ref_i].key = ref[ref_i].value = qToBigEndian(-1);
                ref_i++;
            }
        }
        
        if (ObjectGroup *group = layer->asObjectGroup()) {
            foreach (const MapObject *o, group->objects()) {
                it = o->properties().constBegin();
                it_end = o->properties().constEnd();
                object[objects].x = qToBigEndian((int) o->x());
                object[objects].y = qToBigEndian((int) o->y());
                object[objects].layer = qToBigEndian(layers);
                object[objects].ref = qToBigEndian(ref_i);
                
                for (; it != it_end; it++, ref_i++) {
                    ref[ref_i].key = qToBigEndian(i);
                    i += copyString(it.key().toUtf8().data(), &strings[i]);
                    ref[ref_i].value = qToBigEndian(i);
                    i += copyString(it.value().toUtf8().data(), &strings[i]);
                }
                if (!o->type().isEmpty()) {
                    ref[ref_i].key = qToBigEndian(i);
                    i += copyString((char *) "TYPE", &strings[i]);
                    ref[ref_i].value = qToBigEndian(i);
                    i += copyString(o->type().toUtf8().data(), &strings[i]);
                    ref_i++;
                }
                if (!o->name().isEmpty()) {
                    ref[ref_i].key = qToBigEndian(i);
                    i += copyString((char *) "NAME", &strings[i]);
                    ref[ref_i].value = qToBigEndian(i);
                    i += copyString(o->type().toUtf8().data(), &strings[i]);
                    ref_i++;
                }
                ref[ref_i].key = ref[ref_i].value = qToBigEndian(-1);
                ref_i++;
            }
        }
    }
    
    data.resize(map_s->strtable_len);
    data.insert(0, strings, map_s->strtable_len);
    zdata = compress(data);
    map_s->strtable_zlen = zdata.size();
    fwrite((void *) zdata.data(), zdata.size(), 1, fp);
    
    data.resize(map_s->strtable_refs_len);
    data.insert(0, (const char *) ref, map_s->strtable_refs_len);
    zdata = compress(data);
    map_s->strtable_refs_zlen = zdata.size();
    fwrite((void *) zdata.data(), zdata.size(), 1, fp);
    
    data.resize(map_s->objects * sizeof(LDMZ_OBJECT));
    data.insert(0, (const char *) object, map_s->objects * sizeof(LDMZ_OBJECT));
    zdata = compress(data);
    map_s->object_zlen = zdata.size();
    fwrite((void *) zdata.data(), zdata.size(), 1, fp);
    
    return true;
}


void LDMZPlugin::countStuff(const Tiled::Map *map, unsigned int *layers, unsigned int *objects, unsigned int *refs, unsigned int *stringmem) {
    *layers = *objects = *stringmem = 0;
    Properties::const_iterator it = map->properties().constBegin();
    Properties::const_iterator it_end = map->properties().constEnd();
    
    for (; it != it_end; it++) {
        refs[0]++;
        stringmem[0] += (it.key().toUtf8().size() + 1);
        stringmem[0] += (it.value().toUtf8().size() + 1);
    }
    
    refs[0]++;                          // NULL-terminator ref
    
    foreach (Layer *layer, map->layers()) {
        it = layer->properties().constBegin();
        it_end = layer->properties().constBegin();
        if (layer->isTileLayer()) {
            if (layer->name() == "collision");
            else {
                for (; it != it_end; it++) {
                    refs[0]++;
                    stringmem[0] += (it.key().toUtf8().size() + 1);
                    stringmem[0] += (it.value().toUtf8().size() + 1);
                }
                if (!layer->name().isEmpty()) {
                    stringmem[0] += (layer->name().toUtf8().size() + 2 + strlen("NAME"));
                    refs[0]++;
                }
                refs[0]++;              // NULL-terminator ref
                layers[0]++;
            }
        }
        
        if (ObjectGroup *group = layer->asObjectGroup()) {
            foreach (const MapObject *o, group->objects()) {
                it = o->properties().constBegin();
                it_end = o->properties().constEnd();
                for (; it != it_end; it++) {
                    refs[0]++;
                    stringmem[0] += (it.key().toUtf8().size() + 1);
                    stringmem[0] += (it.value().toUtf8().size() + 1);
                }
                
                if (!o->type().isEmpty()) {
                    stringmem[0] += (o->type().toUtf8().size() + 2 + strlen("TYPE"));
                    refs[0]++;
                }
                
                if (!o->name().isEmpty()) {
                    stringmem[0] += (o->name().toUtf8().size() + 2 + strlen("NAME"));
                    refs[0]++;
                }
                refs[0]++;              // NULL-terminator ref
                objects[0]++;
            }
        }
    }
    
    return;
}


bool LDMZPlugin::write(const Tiled::Map *map, const QString &fileName) {
    FILE *fp;
    unsigned int *layer;
    char *strings;
    int i, j, t, mapWidth, mapHeight, layerSZ, layers;
    QByteArray data, zdata;
    LDMZ_OBJECT *object;
    LDMZ_MAP map_s;
    LDMZ_REF *ref;
    LDMZ_LAYER *layer_s;

    map_s.layers = map_s.objects = map_s.refs = map_s.strtable_len = 0;
    countStuff(map, &map_s.layers, &map_s.objects, &map_s.refs, &map_s.strtable_len);
    map_s.strtable_refs_len = sizeof(LDMZ_REF) * map_s.refs;
    mapWidth = map->width();
    mapHeight = map->height();
    layerSZ = mapWidth * mapHeight;
    layers = map_s.layers;
    
    if ((layer = (unsigned int *) malloc(sizeof(unsigned int) * map->width() * map->height() * layers)) == NULL) {
        mError = tr("Unable to allocate memory for the layer write buffer.");
        return false;
    }

    for (i = 0; i < ((int) map->width() * (int) map->height() * (int) map_s.layers); i++)
        layer[i] = 0;
    if ((fp = fopen(fileName.toUtf8().data(), "w+b")) == NULL) {
        mError = tr("Could not open file for writing.");
        free(layer);
        return false;
    }
    
    object = (LDMZ_OBJECT *) malloc(sizeof(LDMZ_OBJECT) * map_s.objects);
    strings = (char *) malloc(map_s.strtable_len);
    ref = (LDMZ_REF *) malloc(map_s.refs * sizeof(LDMZ_REF));
    layer_s = (LDMZ_LAYER *) malloc(map_s.layers * sizeof(LDMZ_LAYER));
    
    if ((!object && map_s.objects) || (!strings) || (!ref) || (!layer)) {
        mError = tr("Unable to malloc(), you're probably out of RAM");
        goto error;
    }
    
    fwrite((void *) &map, sizeof(LDMZ_MAP), 1, fp);
    
    if (!writeStrings(map, strings, ref, &map_s, object, layer_s, fp))
        goto error;
    free(strings);
    free(ref);
    free(object);

    layers = -1;

    foreach (Layer *t_layer, map->layers()) {
        if (TileLayer *tileLayer = t_layer->asTileLayer()) {
            if (t_layer->name() == "collision") {
                if (layers > -1)
                    for (i = 0; i < mapWidth; i++)
                       for (j = 0; j < mapHeight; j++) {
                           if (Tile *tile = tileLayer->cellAt(i, j).tile) {
                                t = tile->id();
                                t <<= 16;
                                layer[j * mapWidth + i + layerSZ * layers] |= qToBigEndian((qint32)t);
                            }
                        }
            } else {
                layers++;
                for (i = 0; i < mapWidth; i++)
                    for (j = 0; j < mapHeight; j++) {
                        if (Tile *tile = tileLayer->cellAt(i, j).tile) {
                            t = tile->id();
                         } else
                            t = 0;
                        layer[j * mapWidth + i + layerSZ * layers] = qToBigEndian ((qint32)t);
                    }
                
                /* Unsupported by Tiled */
                layer_s[layers].layer_w = qToBigEndian(mapWidth);map->tileWidth();
                layer_s[layers].layer_h = qToBigEndian(mapHeight);
                layer_s[layers].layer_offset_x = qToBigEndian(0);
                layer_s[layers].layer_offset_y = qToBigEndian(0);
                layer_s[layers].tile_w = qToBigEndian(map->tileWidth());
                layer_s[layers].tile_h = qToBigEndian(map->tileHeight());
            }
        }
    }
    
    layers++;

    if (layers < 1) {
        mError = tr("You need to have at least one non-collision tile layer!");
        free(layer); fclose(fp);
        return false;
    }
    
    data.resize(0);
    
    /* I know, this is very slow. But I can't think of a lazier way :P */
    for (i = 0; i < layers; i++) {
        data.insert(0, (char *) &layer[layerSZ * i], layerSZ * sizeof(unsigned int));
        zdata = compress(data);
        layer_s[i].layer_zlen = qToBigEndian(zdata.size());
    }
    
    data.resize(0);
    data.insert(0, (char *) layer_s, sizeof(LDMZ_LAYER) * layers);
    zdata = compress(data);
    fwrite((void *) zdata.data(), zdata.size(), 1, fp);
    map_s.layer_zlen = zdata.size();
    data.resize(0);
    
    for (i = 0; i < layers; i++) {
        data.insert(0, (char *) &layer[layerSZ * i], layerSZ * sizeof(unsigned int));
        zdata = compress(data);
        fwrite((void *) zdata.data(), zdata.size(), 1, fp);
    }
    
    map_s.magic = LDMZ_MAGIC;
    map_s.version = LDMZ_VERSION;
    
    fseek(fp, 0, SEEK_SET);
    writeInts((unsigned int *) &map_s, sizeof(LDMZ_MAP) / 4, fp);
    
    fclose(fp);
    free(layer);

    return true;
    
    error:
        free(object);
        free(strings);
        free(ref);
        free(layer);
        fclose(fp);
        return false;
}

Q_EXPORT_PLUGIN2(LDMZ, LDMZPlugin)
