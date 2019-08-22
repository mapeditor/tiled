/*
 * clipboardmanager.h
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#pragma once

#include "properties.h"

#include <QObject>

#include <memory>

class QClipboard;

namespace Tiled {

class ObjectGroup;
class Map;

class MapDocument;
class MapView;

/**
 * The clipboard manager deals with interaction with the clipboard.
 */
class ClipboardManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ClipboardManager)

    ClipboardManager();

public:
    static ClipboardManager *instance();
    static void deleteInstance();

    bool hasMap() const;
    std::unique_ptr<Map> map() const;
    void setMap(const Map &map);

    bool hasProperties() const;
    Properties properties() const;
    void setProperties(const Properties &properties);

    bool copySelection(const MapDocument &mapDocument);

    enum PasteFlag {
        PasteDefault        = 0x0,
        PasteNoTileObjects  = 0x1,
        PasteInPlace        = 0x2,
    };
    Q_DECLARE_FLAGS(PasteFlags, PasteFlag)
    Q_FLAGS(PasteFlags)

    void pasteObjectGroup(const ObjectGroup *objectGroup,
                          MapDocument *mapDocument,
                          const MapView *view,
                          PasteFlags flags = PasteDefault);

signals:
    void hasMapChanged();
    void hasPropertiesChanged();

private:
    void update();

    QClipboard *mClipboard;
    bool mHasMap;
    bool mHasProperties;

    static ClipboardManager *mInstance;
};

/**
 * Returns whether the clipboard has a map.
 */
inline bool ClipboardManager::hasMap() const
{
    return mHasMap;
}

/**
 * Returns whether the clipboard holds some custom properties.
 */
inline bool ClipboardManager::hasProperties() const
{
    return mHasProperties;
}

} // namespace Tiled

Q_DECLARE_OPERATORS_FOR_FLAGS(Tiled::ClipboardManager::PasteFlags)
