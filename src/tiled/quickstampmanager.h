/*
 * quickstampmanager.h
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
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

#ifndef QUICKSTAMPMANAGER_H
#define QUICKSTAMPMANAGER_H

#include <QObject>
#include <QVector>

namespace Tiled {

class Map;
class TileLayer;

namespace Internal {

class MapDocument;

/**
 * Implements a manager which handles lots of copy&paste slots.
 * Ctrl + <1..9> will store tile layers, and just <1..9> will recall these
 * tile layers.
 */
class QuickStampManager: public QObject
{
    Q_OBJECT

public:
    /**
     * Returns the quick stamp manager instance. Creates the instance when it
     * doesn't exist yet.
     */
    static QuickStampManager *instance();

    /**
     * Deletes the tool manager instance. Should only be called on application
     * exit.
     */
    static void deleteInstance();

    /**
     * Return the keys which should be used.
     * Note: To store a tile layer <Ctrl> is added. The given keys will work
     * for recalling the stored values
     */
    static QList<int> keys() {
        QList<int> keys;
        keys << Qt::Key_1
             << Qt::Key_2
             << Qt::Key_3
             << Qt::Key_4
             << Qt::Key_5
             << Qt::Key_6
             << Qt::Key_7
             << Qt::Key_8
             << Qt::Key_9;
        return keys;
    }

public slots:
    void selectQuickStamp(int index);
    void saveQuickStamp(int index);
    void setMapDocument(MapDocument *mapDocument);

signals:
    void setStampBrush(const TileLayer*);

private:
    Q_DISABLE_COPY(QuickStampManager)

    QuickStampManager();
    ~QuickStampManager();

    void cleanQuickStamps();
    void eraseQuickStamp(int index);

    static QuickStampManager *mInstance;

    QVector<Map*> mQuickStamps;
    MapDocument *mMapDocument;
};

} // namespace Tiled::Internal
} // namespace Tiled

#endif // QUICKSTAMPMANAGER_H
