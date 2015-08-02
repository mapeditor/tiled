/*
 * pluginmanager.h
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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "tiled_global.h"

#include <QList>
#include <QObject>
#include <QString>

namespace Tiled {

struct LoadedPlugin
{
    LoadedPlugin(const QString &fileName, QObject *instance)
        : fileName(fileName)
        , instance(instance)
    {}

    QString fileName;
    QObject *instance;
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
    const QList<LoadedPlugin> &plugins() const { return mPlugins; }

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
        for (QObject *object : mInstance->mObjects)
            if (T *result = qobject_cast<T*>(object))
                results.append(result);
        return results;
    }

    const LoadedPlugin *pluginByFileName(const QString &pluginFileName) const;

signals:
    void objectAdded(QObject *object);
    void objectAboutToBeRemoved(QObject *object);

private:
    Q_DISABLE_COPY(PluginManager)

    PluginManager();
    ~PluginManager();

    static PluginManager *mInstance;

    QList<LoadedPlugin> mPlugins;
    QObjectList mObjects;
};

} // namespace Tiled

#endif // PLUGINMANAGER_H
