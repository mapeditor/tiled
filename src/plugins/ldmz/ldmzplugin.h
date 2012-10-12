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

#ifndef LDMZPLUGIN_H
#define LDMZPLUGIN_H

#define LDMZ_MAGIC              0xFF00E00E
#define LDMZ_VERSION            0x55555555

#include "ldmz_global.h"

#include "mapobject.h"
#include "objectgroup.h"
#include "mapwriterinterface.h"
#include "compression.h"

#include <QObject>
#include <QMap>

typedef struct {
    int                     key;
    int                     value;
} LDMZ_REF;


typedef struct {
    unsigned int            x;
    unsigned int            y;
    unsigned int            layer;
    unsigned int            ref;
} LDMZ_OBJECT;


typedef struct {
    unsigned int            layer_w;
    unsigned int            layer_h;
    int                     layer_offset_x;
    int                     layer_offset_y;
    unsigned int            layer_zlen;
    unsigned int            prop_ref;
    unsigned int            tile_w;
    unsigned int            tile_h;
} LDMZ_LAYER;


typedef struct {
    unsigned int            magic;
    unsigned int            version;
    unsigned int            strtable_len;
    unsigned int            strtable_zlen;
    unsigned int            strtable_refs_len;
    unsigned int            strtable_refs_zlen;
    unsigned int            layers;
    unsigned int            layer_zlen;
    unsigned int            objects;
    unsigned int            object_zlen;
    unsigned int            refs;
    unsigned int            main_ref;
} LDMZ_MAP;


namespace LDMZ {

class CTMBSHARED_EXPORT LDMZPlugin
        : public QObject
        , public Tiled::MapWriterInterface
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapWriterInterface)

public:
    LDMZPlugin();

    // MapWriterInterface
    bool write(const Tiled::Map *map, const QString &fileName);
    QString nameFilter() const;
    QString errorString() const;

private:
    void writeInt(unsigned int val, FILE *fp);
    void writeInts(unsigned int *val, int vals, FILE *fp);
    int copyString(char *string, char *target);
    bool writeStrings(const Tiled::Map *map, char *strings, LDMZ_REF *ref, LDMZ_MAP *map_s, LDMZ_OBJECT *object, LDMZ_LAYER *layer_s, FILE *fp);
    void countStuff(const Tiled::Map *map, unsigned int *layers, unsigned int *objects, unsigned int *refs, unsigned int *stringmem);

    QString mError;
};

} // namespace LDMZ





#endif // LDMZPLUGIN_H
