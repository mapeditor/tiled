/*
 * tilestampmanager.h
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2014-2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilestamp.h"

#include <QMap>
#include <QObject>
#include <QVector>

namespace Tiled {

class Map;
class TileLayer;

class MapDocument;
class TileStamp;
class TileStampModel;
class ToolManager;

/**
 * Implements a manager which handles lots of copy&paste slots.
 * Ctrl + <1..9> will store tile layers, and just <1..9> will recall these
 * tile layers.
 */
class TileStampManager : public QObject
{
    Q_OBJECT

public:
    TileStampManager(const ToolManager &toolManager, QObject *parent = nullptr);
    ~TileStampManager();

    static QList<Qt::Key> quickStampKeys();

    TileStampModel *tileStampModel() const;

public slots:
    TileStamp createStamp();
    void addVariation(const TileStamp &targetStamp);

    void selectQuickStamp(int index);
    void createQuickStamp(int index);
    void extendQuickStamp(int index);

    void stampsDirectoryChanged();

signals:
    void setStamp(const TileStamp &stamp);

private:
    Q_DISABLE_COPY(TileStampManager)

    void eraseQuickStamp(int index);
    void setQuickStamp(int index, TileStamp stamp);

    void loadStamps();

private:
    void stampAdded(TileStamp stamp);
    void stampRenamed(TileStamp stamp);
    void saveStamp(const TileStamp &stamp);
    void deleteStamp(const TileStamp &stamp);

    QVector<TileStamp> mQuickStamps;
    QMap<QString, TileStamp> mStampsByName;
    TileStampModel *mTileStampModel;

    const ToolManager &mToolManager;
};


/**
 * Returns the keys used for quickly accessible tile stamps.
 * Note: To store a tile layer <Ctrl> is added. The given keys will work
 * for recalling the stored values.
 */
inline QList<Qt::Key> TileStampManager::quickStampKeys()
{
    return {
        Qt::Key_1,
        Qt::Key_2,
        Qt::Key_3,
        Qt::Key_4,
        Qt::Key_5,
        Qt::Key_6,
        Qt::Key_7,
        Qt::Key_8,
        Qt::Key_9
    };
}

inline TileStampModel *TileStampManager::tileStampModel() const
{
    return mTileStampModel;
}

} // namespace Tiled
