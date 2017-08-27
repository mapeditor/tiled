/*
 * networkaccessmanager.cpp
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

#include "networkaccessmanager.h"

#include "tile.h"
#include "tilesetmanager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QDebug>

namespace Tiled {

NetworkAccessManager::NetworkAccessManager(QObject *parent)
    : QObject(parent)
    , mNAM(new QNetworkAccessManager(this))
{
    QObject::connect(mNAM, &QNetworkAccessManager::finished,
                     this, &NetworkAccessManager::onFinished);
}

NetworkAccessManager *NetworkAccessManager::instance()
{
    static NetworkAccessManager nam;
    return &nam;
}

void NetworkAccessManager::requestImage(Tileset *tileset)
{
    QNetworkRequest request(tileset->imageSource());
    QNetworkReply *reply = mNAM->get(request);

    mImageReplies.insert(reply, tileset->sharedPointer());
    tileset->setImageStatus(LoadingInProgress);
}

void NetworkAccessManager::requestImage(Tile *tile)
{
    QNetworkRequest request(tile->imageSource());
    QNetworkReply *reply = mNAM->get(request);

    const TileRef ref = { tile->tileset()->sharedPointer(), tile->id() };

    mTileImageReplies.insert(reply, std::move(ref));
    tile->setImageStatus(LoadingInProgress);
}

void NetworkAccessManager::onFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (mImageReplies.contains(reply)) {
        const SharedTileset tileset = mImageReplies.take(reply);

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Failed to download image:" << reply->url() << "\n"
                       << reply->errorString();
            tileset->setImageStatus(LoadingError);
            return;
        }

        QImage image;
        if (!image.loadFromData(reply->readAll())) {
            qWarning() << "Failed to read image:" << reply->url();
            tileset->setImageStatus(LoadingError);
        }

        emit TilesetManager::instance()->tilesetImagesChanged(tileset.data());

        tileset->loadFromImage(image, tileset->imageSource());
    }

    if (mTileImageReplies.contains(reply)) {
        const TileRef ref = mTileImageReplies.take(reply);
        if (Tile *tile = ref.tileset->findTile(ref.tileId)) {
            QImage image;
            if (!image.loadFromData(reply->readAll())) {
                qWarning() << "Failed to read image:" << reply->url();
                tile->setImageStatus(LoadingError);
            }

            ref.tileset->setTileImage(tile, QPixmap::fromImage(image), tile->imageSource());
            tile->setImageStatus(LoadingReady);
            emit TilesetManager::instance()->tilesetImagesChanged(ref.tileset.data());
        }
    }
}

} // namespace Tiled
