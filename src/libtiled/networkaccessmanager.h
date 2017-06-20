/*
 * networkaccessmanager.h
 * Copyright 2017, Your Name <your.name@domain>
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

#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include "tileset.h"

#include <QHash>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

namespace Tiled {

class TILEDSHARED_EXPORT NetworkAccessManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkAccessManager(QObject *parent = nullptr);

    static NetworkAccessManager *instance();

    void requestImage(Tileset *tileset);
    void requestImage(Tile *tile);

private slots:
    void onFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *mNAM;

    struct TileRef {
        SharedTileset tileset;
        int tileId;
    };

    QHash<QNetworkReply*, SharedTileset> mImageReplies;
    QHash<QNetworkReply*, TileRef> mTileImageReplies;
};

} // namespace Tiled

#endif // NETWORKACCESSMANAGER_H
