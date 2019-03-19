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

#include <QObject>
#include <QJSValue>

#include <memory>

class QJSEngine;

namespace Tiled {

class ScriptModule;

class ScriptManager : public QObject
{
    Q_OBJECT

public:
    static ScriptManager &instance();
    static void deleteInstance();

    ScriptModule *module() const;
    QJSEngine *engine() const;

    QJSValue evaluate(const QString &program,
                      const QString &fileName = QString(), int lineNumber = 1);

    QJSValue evaluateFile(const QString &fileName);

    void evaluateStartupScripts();

    void checkError(QJSValue value, const QString &program = QString());
    void throwError(const QString &message);

    void reset();

    void scriptFilesChanged(const QStringList &scriptFiles);

private:
    explicit ScriptManager(QObject *parent = nullptr);

    void initialize();

    QJSEngine *mEngine;
    ScriptModule *mModule;
    FileSystemWatcher mWatcher;

    static std::unique_ptr<ScriptManager> mInstance;
};


inline ScriptModule *ScriptManager::module() const
{
    return mModule;
}

inline QJSEngine *ScriptManager::engine() const
{
    return mEngine;
}

} // namespace Tiled
