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

#include "mapformat.h"
#include "plugin.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QPluginLoader>

namespace Tiled {

PluginManager *PluginManager::mInstance;

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

void PluginManager::addObject(QObject *object)
{
    Q_ASSERT(object);
    Q_ASSERT(mInstance);
    Q_ASSERT(!mInstance->mObjects.contains(object));

    mInstance->mObjects.append(object);
    emit mInstance->objectAdded(object);
}

void PluginManager::removeObject(QObject *object)
{
    Q_ASSERT(object);
    Q_ASSERT(mInstance);
    Q_ASSERT(mInstance->mObjects.contains(object));

    emit mInstance->objectAboutToBeRemoved(object);
    mInstance->mObjects.removeOne(object);
}

void PluginManager::loadPlugins()
{
    // Load static plugins
    foreach (QObject *instance, QPluginLoader::staticInstances()) {
        mPlugins.append(LoadedPlugin(QLatin1String("<static>"), instance));

        if (Plugin *plugin = qobject_cast<Plugin*>(instance))
            plugin->initialize();
        else
            addObject(instance);
    }

    // Determine the plugin path based on the application location
#ifndef TILED_PLUGIN_DIR
    QString pluginPath = QCoreApplication::applicationDirPath();
#endif

#ifdef Q_OS_WIN32
    pluginPath += QLatin1String("/plugins/tiled");
#elif defined(Q_OS_MAC)
    pluginPath += QLatin1String("/../PlugIns");
#elif defined(TILED_PLUGIN_DIR)
    QString pluginPath = QLatin1String(TILED_PLUGIN_DIR);
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

        mPlugins.append(LoadedPlugin(pluginFile, instance));

        if (Plugin *plugin = qobject_cast<Plugin*>(instance))
            plugin->initialize();
        else
            addObject(instance);
    }
}

const LoadedPlugin *PluginManager::pluginByFileName(const QString &pluginFileName) const
{
    foreach (const LoadedPlugin &plugin, mPlugins)
        if (pluginFileName == plugin.fileName)
            return &plugin;

    return 0;
}

} // namespace Tiled
