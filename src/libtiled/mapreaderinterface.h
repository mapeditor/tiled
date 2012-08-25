/*
 * mapreaderinterface.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MAPREADERINTERFACE_H
#define MAPREADERINTERFACE_H

#include <QtPlugin>
#include <QStringList>

class QString;

namespace Tiled {

class Map;

/**
 * An interface to be implemented by map readers. A map reader implements
 * support for loading a certain map format.
 *
 * At the moment, Tiled only provides a reader for its own .tmx map format
 * through the TmxMapReader.
 */
class MapReaderInterface
{
public:
    virtual ~MapReaderInterface() {}

    /**
     * Reads the map and returns a new Map instance, or 0 if reading failed.
     */
    virtual Map *read(const QString &fileName) = 0;

    /**
     * Returns name filters of this map reader, for multiple formats.
     */
    virtual QStringList nameFilters() const { return QStringList(nameFilter()); }

    /**
     * Returns whether this map reader supports reading the given file.
     *
     * Generally would do a file extension check.
     */
    virtual bool supportsFile(const QString &fileName) const = 0;

    /**
     * Returns the error to be shown to the user if an error occured while
     * trying to read a map.
     */
    virtual QString errorString() const = 0;

protected:
    /**
     * Returns the name filter of this map reader.
     *
     * Protected because it should not be used outside the plugin since
     * plugins may expose multiple name filters. This thing exists only for
     * convenience for plugin writers since most plugins will have only one
     * name filter.
     */
    virtual QString nameFilter() const { return QString(); }
};

} // namespace Tiled

Q_DECLARE_INTERFACE(Tiled::MapReaderInterface,
                    "org.mapeditor.MapReaderInterface")

#endif // MAPREADERINTERFACE_H
