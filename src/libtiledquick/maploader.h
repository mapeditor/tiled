/*
 * maploader.h
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled Quick.
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

#include "mapref.h"
#include "tiledquick_global.h"
#include "editablemap.h"

#include <QObject>
#include <QUrl>

#include <memory>

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT MapLoader : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(TiledQuick::MapRef map READ map NOTIFY sourceChanged)
    Q_PROPERTY(Tiled::EditableMap* editableMap READ editableMap NOTIFY sourceChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

public:
    enum Status {
        Null,
        Ready,
        Error
    };
    Q_ENUM(Status)

    explicit MapLoader(QObject *parent = nullptr);
    ~MapLoader() override;

    QUrl source() const;
    MapRef map() const;
    Tiled::EditableMap* editableMap() const;
    Status status() const;
    QString error() const;

signals:
    void sourceChanged(const QUrl &source);
    void mapChanged(Tiled::Map *map);
    void statusChanged(Status status);
    void errorChanged(const QString &error);

public slots:
    void setSource(const QUrl &source);

private:
    QUrl m_source;
    std::unique_ptr<Tiled::EditableMap> m_editableMap;
    Status m_status;
    QString m_error;
};


inline MapRef MapLoader::map() const
{
    if (m_editableMap)
        return m_editableMap->map();
    else
        return nullptr;
}

inline Tiled::EditableMap* MapLoader::editableMap() const
{
    return m_editableMap.get();
}

inline MapLoader::Status MapLoader::status() const
{
    return m_status;
}

inline QString MapLoader::error() const
{
    return m_error;
}

inline QUrl MapLoader::source() const
{
    return m_source;
}

} // namespace TiledQuick
