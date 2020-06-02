/*
 * tiledapplication.cpp
 * Copyright 2011-2020, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "documentmanager.h"
#include "languagemanager.h"
#include "pluginmanager.h"
#include "preferences.h"
#include "scriptmanager.h"
#include "templatemanager.h"
#include "tilesetmanager.h"

#include <QFileOpenEvent>
#include <QJsonArray>
#include <QJsonDocument>

using namespace Tiled;

#define STRINGIFY(x) #x
#define AS_STRING(x) STRINGIFY(x)

TiledApplication::TiledApplication(int &argc, char **argv)
    : QtSingleApplication(argc, argv)
{
    setOrganizationDomain(QLatin1String("mapeditor.org"));
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    setApplicationName(QLatin1String("Tiled"));
#else
    setApplicationName(QLatin1String("tiled"));
#endif
    setApplicationDisplayName(QLatin1String("Tiled"));
    setApplicationVersion(QLatin1String(AS_STRING(TILED_VERSION)));

    LanguageManager::instance()->installTranslators();

    connect(this, &QtSingleApplication::messageReceived,
            this, &TiledApplication::onMessageReceived);
}

TiledApplication::~TiledApplication()
{
    TemplateManager::deleteInstance();
    ScriptManager::deleteInstance();
    TilesetManager::deleteInstance();
    Preferences::deleteInstance();
    PluginManager::deleteInstance();
}

bool TiledApplication::event(QEvent *event)
{
    // Handle the QFileOpenEvent to open files on MacOS X.
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *fileOpenEvent = static_cast<QFileOpenEvent*>(event);
        emit fileOpenRequest(fileOpenEvent->file());
        return true;
    }
    return QApplication::event(event);
}

void TiledApplication::onMessageReceived(const QString &message)
{
    // Open files requested by another instance of Tiled
    const QJsonArray files = QJsonDocument::fromJson(message.toLatin1()).array();
    for (const QJsonValue &file : files)
        emit fileOpenRequest(file.toString());
}
