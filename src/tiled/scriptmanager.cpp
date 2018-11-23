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
#include "scriptmodule.h"

#include <QJSEngine>
#include <QtDebug>

namespace Tiled {
namespace Internal {

ScriptManager &ScriptManager::instance()
{
    static ScriptManager scriptManager;
    return scriptManager;
}

ScriptManager::ScriptManager(QObject *parent)
    : QObject(parent)
    , mJSEngine(new QJSEngine(this))
{
#if QT_VERSION < 0x050600
    mJSEngine->installTranslatorFunctions();
#else
    mJSEngine->installExtensions(QJSEngine::AllExtensions);
#endif

    ScriptModule *module = new ScriptModule(this);

    QJSValue globalObject = mJSEngine->globalObject();
    globalObject.setProperty(QStringLiteral("tiled"), mJSEngine->newQObject(module));
}

QJSValue ScriptManager::evaluate(const QString &program,
                                 const QString &fileName, int lineNumber)
{
    QJSValue result = mJSEngine->evaluate(program, fileName, lineNumber);
    if (result.isError()) {
        qDebug() << "Uncaught exception at line"
                 << result.property(QLatin1String("lineNumber")).toInt()
                 << ":" << result.toString();
    }
    return result;
}

} // namespace Internal
} // namespace Tiled
