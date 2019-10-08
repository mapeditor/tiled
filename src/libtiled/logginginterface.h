/*
 * logginginterface.h
 * Copyright 2013, Samuli Tuomola <samuli.tuomola@gmail.com>
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include "tiled_global.h"

#include <QObject>
#include <QPoint>
#include <QWeakPointer>

#include <functional>

class QString;

namespace Tiled {

class Layer;
class Map;
class MapObject;
class Object;
class Tile;
class Tileset;

class TILEDSHARED_EXPORT Issue
{
public:
    enum Severity {
        Error,
        Warning
    };

    Issue();
    Issue(Severity severity,
          const QString &text,
          const std::function<void()> &callback = std::function<void()>(),
          const void *context = nullptr);

    Severity severity() const { return mSeverity; }
    QString text() const { return mText; }

    std::function<void()> callback() const { return mCallback; }
    void setCallback(std::function<void()> callback);

    void setContext(const void *context) { mContext = context; }
    const void *context() const { return mContext; }

    unsigned id() const { return mId; }

    void addOccurrence(const Issue &issue);
    int occurrences() const { return mOccurrences; }

    bool operator==(const Issue &o) const
    {
        return severity() == o.severity()
                && text() == o.text();
    }

private:
    Issue::Severity mSeverity = Issue::Error;
    QString mText;
    std::function<void()> mCallback;
    const void *mContext = nullptr;

    int mOccurrences = 1;
    unsigned mId = 0;

    static unsigned mNextIssueId;
};

/**
 * An interface for reporting issues.
 *
 * Normally you'd use the convenience functions in the Tiled namespace.
 */
class TILEDSHARED_EXPORT LoggingInterface : public QObject
{
    Q_OBJECT

    explicit LoggingInterface(QObject *parent = nullptr);

public:
    static LoggingInterface &instance();

    enum OutputType {
        INFO,
        WARNING,
        ERROR
    };

    void report(const Issue &issue);
    void log(OutputType type, const QString &message);

signals:
    void issue(const Issue &issue);

    void info(const QString &message);
    void warning(const QString &message);
    void error(const QString &message);

    void removeIssuesWithContext(const void *context);
};

inline void REPORT(const Issue &issue)
{
    LoggingInterface::instance().report(issue);
}

inline void INFO(const QString &message)
{
    LoggingInterface::instance().log(LoggingInterface::INFO, message);
}

inline void WARNING(const QString &message, std::function<void()> callback = std::function<void()>(), const void *context = nullptr)
{
    REPORT(Issue { Issue::Warning, message, callback, context });
}

inline void ERROR(const QString &message, std::function<void()> callback = std::function<void()>(), const void *context = nullptr)
{
    REPORT(Issue { Issue::Error, message, callback, context });
}

inline void INFO(QLatin1String message)
{
    INFO(QString(message));
}

inline void WARNING(QLatin1String message, std::function<void()> callback = std::function<void()>(), const void *context = nullptr)
{
    WARNING(QString(message), callback, context);
}

inline void ERROR(QLatin1String message, std::function<void()> callback = std::function<void()>(), const void *context = nullptr)
{
    ERROR(QString(message), callback, context);
}

// TODO: Try "static inline" once we switch to C++17
#define ACTIVATABLE(Class) \
    void operator() () const { activated(*this); } \
    static std::function<void (const Class &)> activated;

struct TILEDSHARED_EXPORT OpenFile
{
    QString file;

    ACTIVATABLE(OpenFile)
};

struct TILEDSHARED_EXPORT JumpToTile
{
    JumpToTile(const Map *map, QPoint tilePos, const Layer *layer = nullptr);

    QString mapFile;
    QPoint tilePos;
    int layerId = -1;

    ACTIVATABLE(JumpToTile)
};

struct TILEDSHARED_EXPORT JumpToObject
{
    JumpToObject(const MapObject *object);

    QString mapFile;
    int objectId;

    ACTIVATABLE(JumpToObject)
};

struct TILEDSHARED_EXPORT SelectLayer
{
    SelectLayer(const Layer *layer);

    QString mapFile;
    int layerId;

    ACTIVATABLE(SelectLayer)
};

struct TILEDSHARED_EXPORT SelectCustomProperty
{
    SelectCustomProperty(QString fileName, QString propertyName, const Object *object);

    QString fileName;
    QString propertyName;
    int objectType;         // see Object::TypeId
    int id = -1;

    ACTIVATABLE(SelectCustomProperty)
};

struct TILEDSHARED_EXPORT SelectTile
{
    SelectTile(const Tile *tile);

    QWeakPointer<Tileset> tileset;
    QString tilesetFile;
    int tileId;

    ACTIVATABLE(SelectTile)
};

#undef ACTIVATABLE

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Issue)
