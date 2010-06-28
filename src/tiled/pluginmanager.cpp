/*
 * pluginmanager.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "pluginmanager.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QPluginLoader>

using namespace Tiled;
using namespace Tiled::Internal;

PluginManager *PluginManager::mInstance = 0;

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
}

PluginManager *PluginManager::instance()
{
    if (!mInstance)
        mInstance = new PluginManager;

    return mInstance;
}

void PluginManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

void PluginManager::loadPlugins()
{
    // Load static plugins
    foreach (QObject *instance, QPluginLoader::staticInstances())
        mPlugins.append(Plugin(QLatin1String("<static>"), instance));

    // Determine the plugin path based on the application location
    QString pluginPath = QCoreApplication::applicationDirPath();
#ifdef Q_OS_WIN32
    pluginPath += QLatin1String("/plugins/tiled");
#elif defined(Q_OS_MAC)
    pluginPath += QLatin1String("/../PlugIns");
#else
    pluginPath += QLatin1String("/../lib/tiled/plugins");
#endif

    // Load dynamic plugins
    QDirIterator iterator(pluginPath, QDir::Files | QDir::Readable);
    while (iterator.hasNext()) {
        const QString &pluginFile = iterator.next();
        if (!QLibrary::isLibrary(pluginFile))
            continue;

        QPluginLoader loader(pluginFile);
        QObject *instance = loader.instance();

        if (!instance) {
            qWarning() << "Error:" << qPrintable(loader.errorString());
            continue;
        }

        mPlugins.append(Plugin(pluginFile, instance));
    }
}
