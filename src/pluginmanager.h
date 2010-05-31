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

#include <QList>
#include <QObject>
#include <QString>

namespace Tiled {
namespace Internal {

/**
 * A loaded plugin.
 */
struct Plugin
{
    Plugin(const QString &fileName, QObject *instance)
        : fileName(fileName)
        , instance(instance)
    {}

    QString fileName;
    QObject *instance;
};

/**
 * The plugin manager loads the plugins and provides ways to access them.
 */
class PluginManager
{
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
    const QList<Plugin> &plugins() const { return mPlugins; }

    /**
     * Returns the list of plugins that implement a given interface.
     */
    template<typename T> QList<T*> interfaces() const
    {
        QList<T*> results;
        foreach (const Plugin &plugin, mPlugins)
            if (T *result = qobject_cast<T*>(plugin.instance))
                results.append(result);
        return results;
    }

private:
    Q_DISABLE_COPY(PluginManager)

    PluginManager();
    ~PluginManager();

    static PluginManager *mInstance;

    QList<Plugin> mPlugins;
};

} // namespace Internal
} // namespace Tiled

#endif // PLUGINMANAGER_H
