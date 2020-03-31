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

#include <QDir>
#include <QPointF>
#include <QSettings>
#include <QSize>
#include <QStringList>
#include <QVariantMap>

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
    if (fileName.isEmpty())
        return QString();
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
                   map.value(QLatin1String("x")).toReal());
}

template<>
inline QVariant toSettingsValue<QPointF>(const QPointF &point)
{
    return QVariantMap {
        { QLatin1String("x"), point.x() },
        { QLatin1String("y"), point.y() }
    };
}


class Session : protected FileHelper
{
    std::unique_ptr<QSettings> settings;

public:
    explicit Session(const QString &fileName = QString());

    bool save();

    QString fileName() const;
    void setFileName(const QString &fileName);

    void addRecentFile(const QString &fileName);

    QVariantMap fileState(const QString &fileName) const;
    void setFileState(const QString &fileName, const QVariantMap &fileState);

    template <typename T>
    T get(const char *key, const T &defaultValue = T()) const
    { return fromSettingsValue<T>(settings->value(QLatin1String(key), toSettingsValue(defaultValue))); }

    template <typename T>
    void set(const char *key, const T &value) const { settings->setValue(QLatin1String(key), toSettingsValue(value)); }

    bool isSet(const char *key) const { return settings->contains(QLatin1String(key)); }

    static QString defaultFileName();
    static QString defaultFileNameForProject(const QString &projectFile);
    static Session &current();

    QString project;
    QStringList recentFiles;
    QStringList openFiles;
    QStringList expandedProjectPaths;
    QString activeFile;
    QVariantMap fileStates;
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

    inline T get() const;
    inline void set(const T &value);

    inline operator T() const { return get(); }
    inline SessionOption &operator =(const T &value) { set(value); return *this; }

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
    Session::current().set(mKey, value);
}

} // namespace Tiled
