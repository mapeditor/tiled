/*
 * pluginmanager.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

QString PluginFile::fileName() const
{
    if (loader)
        return loader->fileName();

    return QStringLiteral("<static>");
}

bool PluginFile::hasError() const
{
    if (instance)
        return false;

    return state == PluginEnabled || (defaultEnable && state == PluginDefault);
}

QString PluginFile::errorString() const
{
    if (loader)
        return loader->errorString();

    return QString();
}


PluginManager *PluginManager::mInstance;

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
}

/**
 * Sets the enabled state of the given plugin to be explicitly enabled or
 * disabled or to use the default state.
 *
 * Returns whether the change was successful.
 */
bool PluginManager::setPluginState(const QString &fileName, PluginState state)
{
    if (state == PluginDefault)
        mPluginStates.remove(fileName);
    else
        mPluginStates.insert(fileName, state);

    PluginFile *plugin = pluginByFileName(fileName);
    if (!plugin)
        return false;

    plugin->state = state;

    bool loaded = plugin->instance != nullptr;
    bool enable = state == PluginEnabled || (plugin->defaultEnable && state != PluginDisabled);
    bool success = false;

    if (enable && !loaded) {
        success = loadPlugin(plugin);
    } else if (!enable && loaded) {
        success = unloadPlugin(plugin);
    } else {
        success = true;
    }

    return success;
}

bool PluginManager::loadPlugin(PluginFile *plugin)
{
    plugin->instance = plugin->loader->instance();

    if (plugin->instance) {
        if (Plugin *p = qobject_cast<Plugin*>(plugin->instance))
            p->initialize();
        else
            addObject(plugin->instance);

        return true;
    } else {
        qWarning() << "Error:" << qPrintable(plugin->loader->errorString());
        return false;
    }
}

bool PluginManager::unloadPlugin(PluginFile *plugin)
{
    bool derivedPlugin = qobject_cast<Plugin*>(plugin->instance) != nullptr;

    if (plugin->loader->unload()) {
        if (!derivedPlugin)
            removeObject(plugin->instance);

        plugin->instance = nullptr;
        return true;
    } else {
        return false;
    }
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
    mInstance = nullptr;
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
    if (!mInstance)
        return;

    Q_ASSERT(object);
    Q_ASSERT(mInstance->mObjects.contains(object));

    emit mInstance->objectAboutToBeRemoved(object);
    mInstance->mObjects.removeOne(object);
}

void PluginManager::loadPlugins()
{
    // Load static plugins
    const QObjectList &staticPluginInstances = QPluginLoader::staticInstances();
    for (QObject *instance : staticPluginInstances) {
        mPlugins.append(PluginFile(PluginStatic, instance));

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

        QString fileName = QFileInfo(pluginFile).fileName();
        PluginState state = mPluginStates.value(fileName);

        auto *loader = new QPluginLoader(pluginFile, this);
        auto metaData = loader->metaData().value(QStringLiteral("MetaData")).toObject();
        bool defaultEnable = metaData.value(QStringLiteral("defaultEnable")).toBool();

        bool enable = state == PluginEnabled || (defaultEnable && state != PluginDisabled);

        QObject *instance = nullptr;

        if (enable) {
            instance = loader->instance();

            if (!instance)
                qWarning() << "Error:" << qPrintable(loader->errorString());
        }

        mPlugins.append(PluginFile(state, instance, loader, defaultEnable));

        if (!instance)
            continue;

        if (Plugin *plugin = qobject_cast<Plugin*>(instance))
            plugin->initialize();
        else
            addObject(instance);
    }
}

PluginFile *PluginManager::pluginByFileName(const QString &fileName)
{
    for (PluginFile &plugin : mPlugins)
        if (plugin.loader && QFileInfo(plugin.loader->fileName()).fileName() == fileName)
            return &plugin;

    return nullptr;
}

} // namespace Tiled
