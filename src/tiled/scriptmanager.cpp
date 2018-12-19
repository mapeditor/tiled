/*
 * scriptmanager.cpp
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

#include "scriptmanager.h"

#include "documentmanager.h"
#include "editablemap.h"
#include "editabletilelayer.h"
#include "editabletileset.h"
#include "regionvaluetype.h"
#include "scriptmodule.h"
#include "tilelayer.h"

#include <QFile>
#include <QQmlEngine>
#include <QStandardPaths>
#include <QTextCodec>
#include <QtDebug>

namespace Tiled {

std::unique_ptr<ScriptManager> ScriptManager::mInstance;

ScriptManager &ScriptManager::instance()
{
    if (!mInstance)
        mInstance.reset(new ScriptManager);
    return *mInstance;
}

void ScriptManager::deleteInstance()
{
    mInstance.reset();
}

/*
 * mJSEngine needs to be QQmlEngine for the "Qt" module to be available, which
 * is necessary to pass things like QSize or QPoint to some API functions
 * (using Qt.size and Qt.point).
 *
 * It also means we don't need to call QJSEngine::installExtensions, since the
 * QQmlEngine seems to include those by default.
 */

ScriptManager::ScriptManager(QObject *parent)
    : QObject(parent)
    , mEngine(new QQmlEngine(this))
    , mModule(new ScriptModule(this))
{
    qRegisterMetaType<EditableAsset*>();
    qRegisterMetaType<EditableLayer*>();
    qRegisterMetaType<EditableMap*>();
    qRegisterMetaType<EditableTileLayer*>();
    qRegisterMetaType<EditableTileset*>();
    qRegisterMetaType<Cell>();
    qRegisterMetaType<RegionValueType>();

    QJSValue globalObject = mEngine->globalObject();
    globalObject.setProperty(QStringLiteral("tiled"), mEngine->newQObject(mModule));
}

QJSValue ScriptManager::evaluate(const QString &program,
                                 const QString &fileName, int lineNumber)
{
    QJSValue result = mEngine->evaluate(program, fileName, lineNumber);
    if (result.isError()) {
        qDebug().nospace().noquote()
                << "Uncaught exception at line "
                << result.property(QLatin1String("lineNumber")).toInt()
                << ": " << result.toString();
    }
    return result;
}

QJSValue ScriptManager::evaluateFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Error opening file:" << fileName;
        return QJSValue();
    }

    const QByteArray text = file.readAll();
    const QString script = QTextCodec::codecForUtfText(text)->toUnicode(text);

    return evaluate(script, fileName);
}

void ScriptManager::evaluateStartupScripts()
{
    const QStringList configLocations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    for (const QString &configLocation : configLocations) {
        const QString scriptFile = configLocation + QLatin1String("/startup.js");
        if (QFile::exists(scriptFile)) {
            qDebug() << "Evaluating startup script:" << scriptFile;
            evaluateFile(scriptFile);
        }
    }
}

void ScriptManager::throwError(const QString &message)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    module()->error(message);
#else
    engine()->throwError(message);
#endif
}

} // namespace Tiled
