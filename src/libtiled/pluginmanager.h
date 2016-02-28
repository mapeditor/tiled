/*
 * pluginmanager.h
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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "tiled_global.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

#include <functional>

class QPluginLoader;

namespace Tiled {

enum PluginState
{
    PluginDefault,
    PluginEnabled,
    PluginDisabled,
    PluginStatic
};

struct TILEDSHARED_EXPORT PluginFile
{
    PluginFile(PluginState state,
               QObject *instance,
               QPluginLoader *loader = nullptr,
               bool defaultEnable = true)
        : state(state)
        , instance(instance)
        , loader(loader)
        , defaultEnable(defaultEnable)
    {}

    QString fileName() const;
    bool hasError() const;
    QString errorString() const;

    PluginState state;
    QObject *instance;
    QPluginLoader *loader;
    bool defaultEnable;
};


/**
 * The plugin manager loads the plugins and provides ways to access them.
 */
class TILEDSHARED_EXPORT PluginManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Returns the plugin manager instance.
     */
    static PluginManager *instance();

    static void deleteInstance();

    /**
     * Scans the plugin directory for plugins and attempts to load them.
     */
    void loadPlugins();

    /**
     * Returns the list of plugins found by the plugin manager.
     */
    const QList<PluginFile> &plugins() const { return mPlugins; }

    /**
     * Adds the given \a object. This allows the object to be found later based
     * on the interfaces it implements.
     */
    static void addObject(QObject *object);

    /**
     * Removes the given \a object.
     */
    static void removeObject(QObject *object);

    /**
     * Returns the list of objects that implement a given interface.
     */
    template<typename T>
    static QList<T*> objects()
    {
        QList<T*> results;
        if (mInstance)
            for (QObject *object : mInstance->mObjects)
                if (T *result = qobject_cast<T*>(object))
                    results.append(result);
        return results;
    }

    /**
     * Calls the given function for each object implementing a given interface.
     */
    template<typename T>
    static void each(std::function<void(T*)> function)
    {
        if (mInstance)
            for (QObject *object : mInstance->mObjects)
                if (T *result = qobject_cast<T*>(object))
                    function(result);
    }

    PluginFile *pluginByFileName(const QString &fileName);

    const QMap<QString, PluginState> &pluginStates() const;
    bool setPluginState(const QString &fileName, PluginState state);

signals:
    void objectAdded(QObject *object);
    void objectAboutToBeRemoved(QObject *object);

private:
    Q_DISABLE_COPY(PluginManager)

    PluginManager();
    ~PluginManager();

    bool loadPlugin(PluginFile *plugin);
    bool unloadPlugin(PluginFile *plugin);

    static PluginManager *mInstance;

    QList<PluginFile> mPlugins;
    QMap<QString, PluginState> mPluginStates;
    QObjectList mObjects;
};


inline const QMap<QString, PluginState> &PluginManager::pluginStates() const
{
    return mPluginStates;
}

} // namespace Tiled

#endif // PLUGINMANAGER_H
