/*
 * scriptmanager.h
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "filesystemwatcher.h"
#include "tilededitor_global.h"

#include <QJSValue>
#include <QObject>
#include <QQmlError>
#include <QScopedValueRollback>
#include <QStringList>

class QJSEngine;

namespace Tiled {

class ScriptModule;

/**
 * Singleton for managing the script engine and module.
 *
 * Dependencies: ProjectManager, DocumentManager (optional)
 */
class TILED_EDITOR_EXPORT ScriptManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool projectExtensionsSuppressed READ projectExtensionsSuppressed NOTIFY projectExtensionsSuppressedChanged)

public:
    /**
     * While a reset blocker instance exists, resetting of the script engine
     * is suppressed. This avoids a crash when a reset happens while script
     * execution is paused by a popup.
     */
    struct ResetBlocker : QScopedValueRollback<bool> {
        ResetBlocker()
            : QScopedValueRollback<bool>(ScriptManager::instance().mResetBlocked, true)
        {}
    };

    static ScriptManager &instance();
    static void deleteInstance();

    void ensureInitialized();

    void setScriptArguments(const QStringList &arguments);

    const QString &extensionsPath() const;

    ScriptModule *module() const;
    QJSEngine *engine() const;

    QJSValue evaluate(const QString &program,
                      const QString &fileName = QString(), int lineNumber = 1);

    void evaluateFileOrLoadModule(const QString &fileName);

    /**
     * Create a new global identifier ($0, $1, $2, ...) for the value. Returns
     * the name of the identifier.
     */
    QString createTempValue(const QJSValue &value);

    bool checkError(QJSValue value, const QString &program = QString());
    void throwError(const QString &message);
    void throwNullArgError(int argNumber);

    void refreshExtensionsPaths();

    bool projectExtensionsSuppressed() const;
    void enableProjectExtensions();

signals:
    void projectExtensionsSuppressedChanged(bool);

private:
    explicit ScriptManager(QObject *parent = nullptr);
    ~ScriptManager() override = default;

    void reset();
    void initialize();

    void onScriptWarnings(const QList<QQmlError> &warnings);

    void scriptFilesChanged(const QStringList &scriptFiles);

    void loadExtensions();
    void loadExtension(const QString &path);

    QJSValue evaluateFile(const QString &fileName);

    QJSEngine *mEngine = nullptr;
    ScriptModule *mModule = nullptr;
    FileSystemWatcher mWatcher;
    QString mExtensionsPath;
    QStringList mExtensionsPaths;
    int mTempCount = 0;
    bool mProjectExtensionsSuppressed = false;

    friend struct ResetBlocker;
    bool mResetBlocked = false;
    QTimer mResetTimer;

    static ScriptManager *mInstance;
};


inline const QString &ScriptManager::extensionsPath() const
{
    return mExtensionsPath;
}

inline ScriptModule *ScriptManager::module() const
{
    return mModule;
}

inline QJSEngine *ScriptManager::engine() const
{
    return mEngine;
}

inline bool ScriptManager::projectExtensionsSuppressed() const
{
    return mProjectExtensionsSuppressed;
}

} // namespace Tiled
