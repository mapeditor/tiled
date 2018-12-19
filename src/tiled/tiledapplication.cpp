/*
 * tiledapplication.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tiledapplication.h"

#include <QFileOpenEvent>
#include <QJsonArray>
#include <QJsonDocument>

using namespace Tiled;

TiledApplication::TiledApplication(int &argc, char **argv) :
    QtSingleApplication(argc, argv)
{
    connect(this, &TiledApplication::messageReceived,
            this, &TiledApplication::onMessageReceived);
}

bool TiledApplication::event(QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *fileOpenEvent = static_cast<QFileOpenEvent*>(event);
        emit fileOpenRequest(fileOpenEvent->file());
        return true;
    }
    return QApplication::event(event);
}

void TiledApplication::onMessageReceived(const QString &message)
{
   const QJsonArray files = QJsonDocument::fromJson(message.toLatin1()).array();
   for (const QJsonValue &file : files) {
       emit fileOpenRequest(file.toString());
   }
}
