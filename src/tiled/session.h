/*
 * session.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include "tilededitor_global.h"

#include <QDir>
#include <QHash>
#include <QPointF>
#include <QSet>
#include <QSettings>
#include <QSize>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>
#include <QVariantMap>

#include <list>
#include <memory>

namespace Tiled {

class FileHelper
{
public:
    FileHelper(const QString &fileName);

    void setFileName(const QString &fileName);

    QString relative(const QString &fileName) const;
    QStringList relative(const QStringList &fileNames) const;

    QString resolve(const QString &fileName) const;
    QStringList resolve(const QStringList &fileNames) const;

protected:
    QDir mDir;
};

inline QString FileHelper::relative(const QString &fileName) const
{
    if (fileName.startsWith(mDir.path()))
        return mDir.relativeFilePath(fileName);
    return fileName;
}

inline QString FileHelper::resolve(const QString &fileName) const
{
    if (fileName.isEmpty() || fileName.startsWith(QLatin1String("ext:")))
        return fileName;
    return QDir::cleanPath(mDir.filePath(fileName));
}


template<typename T>
T fromSettingsValue(const QVariant &value)
{
    return value.value<T>();
}

template<typename T>
QVariant toSettingsValue(const T &value)
{
    return QVariant::fromValue(value);
}

template<>
inline QSize fromSettingsValue<QSize>(const QVariant &value)
{
    const auto map = value.toMap();
    return QSize(map.value(QLatin1String("width")).toInt(),
                 map.value(QLatin1String("height")).toInt());
}

template<>
inline QVariant toSettingsValue<QSize>(const QSize &size)
{
    return QVariantMap {
        { QLatin1String("width"), size.width() },
        { QLatin1String("height"), size.height() }
    };
}

template<>
inline QPointF fromSettingsValue<QPointF>(const QVariant &value)
{
    const auto map = value.toMap();
    return QPointF(map.value(QLatin1String("x")).toReal(),
                   map.value(QLatin1String("y")).toReal());
}

template<>
inline QVariant toSettingsValue<QPointF>(const QPointF &point)
{
    return QVariantMap {
        { QLatin1String("x"), point.x() },
        { QLatin1String("y"), point.y() }
    };
}

template<>
inline QSet<int> fromSettingsValue<QSet<int>>(const QVariant &value)
{
    const auto variantList = value.toList();
    QSet<int> set;
    for (const auto &variantValue : variantList)
        set.insert(variantValue.value<int>());
    return set;
}

template<>
inline QVariant toSettingsValue<QSet<int>>(const QSet<int> &set)
{
    QVariantList variantList;
    variantList.reserve(set.size());
    for (const int value : set)
        variantList.append(value);
    return variantList;
}


class TILED_EDITOR_EXPORT Session : protected FileHelper
{
    std::unique_ptr<QSettings> settings;

public:
    explicit Session(const QString &fileName);
    ~Session();

    bool save();

    QString fileName() const;
    void setFileName(const QString &fileName);

    void setProject(const QString &fileName);

    void addRecentFile(const QString &fileName);
    void clearRecentFiles();

    void setOpenFiles(const QStringList &fileNames);
    void setActiveFile(const QString &fileName);

    QVariantMap fileState(const QString &fileName) const;
    void setFileState(const QString &fileName, const QVariantMap &fileState);
    void setFileStateValue(const QString &fileName, const QString &name, const QVariant &value);

    enum FileType {
        ExecutablePath,
        ExportedFile,
        ExternalTileset,
        ImageFile,
        ObjectTemplateFile,
        PropertyTypesFile,
        WorkingDirectory,
        WorldFile,
        ShortcutSettingsFile,
    };

    QString lastPath(FileType fileType,
                     QStandardPaths::StandardLocation fallback = QStandardPaths::DocumentsLocation) const;
    void setLastPath(FileType fileType, const QString &path);

    template <typename T>
    T get(const char *key, const T &defaultValue = T()) const
    {
        return fromSettingsValue<T>(settings->value(QLatin1String(key),
                                                    toSettingsValue(defaultValue)));
    }

    template <typename T>
    void set(const char *key, const T &value) const
    {
        const QLatin1String latin1Key(key);
        const QString stringKey(latin1Key);
        const auto settingsValue = toSettingsValue(value);
        if (settings->value(stringKey) == settingsValue)
            return;

        settings->setValue(stringKey, settingsValue);

        const auto it = Session::mChangedCallbacks.constFind(latin1Key);
        if (it != Session::mChangedCallbacks.constEnd())
            for (const auto &cb : it.value())
                cb();
    }

    bool isSet(const char *key) const
    {
        return settings->contains(QLatin1String(key));
    }

    static QString defaultFileName();
    static QString defaultFileNameForProject(const QString &projectFile);

    static Session &initialize();
    static Session &current();
    static Session &switchCurrent(const QString &fileName);
    static void deinitialize();

    QString project;
    QStringList recentFiles;
    QStringList openFiles;
    QStringList expandedProjectPaths;
    QString activeFile;
    QMap<QString, QVariantMap> fileStates;

    using ChangedCallback = std::function<void()>;
    using Callbacks = std::list<ChangedCallback>;
    using CallbackIterator = Callbacks::iterator;

private:
    template<typename T> friend class SessionOption;

    void scheduleSync() { mSyncSettingsTimer.start(); }
    void sync();

    QTimer mSyncSettingsTimer;

    static std::unique_ptr<Session> mCurrent;
    static QHash<QLatin1String, Callbacks> mChangedCallbacks;
};

inline QString Session::fileName() const
{
    return settings->fileName();
}


template<typename T>
class SessionOption
{
public:
    SessionOption(const char * const key, T defaultValue = T())
        : mKey(key)
        , mDefault(defaultValue)
    {}

    T get() const;
    void set(const T &value);

    operator T() const { return get(); }
    SessionOption &operator =(const T &value) { set(value); return *this; }

    Session::CallbackIterator onChange(const Session::ChangedCallback &callback);
    void unregister(Session::CallbackIterator it);

private:
    const char * const mKey;
    const T mDefault;
};

template<typename T>
T SessionOption<T>::get() const
{
    return Session::current().get<T>(mKey, mDefault);
}

template<typename T>
void SessionOption<T>::set(const T &value)
{
    if (get() == value)
        return;

    Session::current().set(mKey, value);
}

template<typename T>
Session::CallbackIterator SessionOption<T>::onChange(const Session::ChangedCallback &callback)
{
    Session::Callbacks &callbacks = Session::mChangedCallbacks[QLatin1String(mKey)];
    callbacks.push_front(callback);
    return callbacks.begin();
}

template<typename T>
void SessionOption<T>::unregister(Session::CallbackIterator it)
{
    Session::Callbacks &callbacks = Session::mChangedCallbacks[QLatin1String(mKey)];
    callbacks.erase(it);
}

} // namespace Tiled
