/*
 * consoledialog.cpp
 * Copyright 2013, Samuli Tuomola <samuli.tuomola@gmail.com>
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

#include "consoledock.h"

#include "logginginterface.h"
#include "pluginmanager.h"

#include <QVBoxLayout>

namespace Tiled {
namespace Internal {

ConsoleDock::ConsoleDock(QWidget *parent)
    : QDockWidget(parent)
{
    setObjectName(QLatin1String("ConsoleDock"));

    setWindowTitle(tr("Debug Console"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);

    plainTextEdit = new QPlainTextEdit;
    plainTextEdit->setReadOnly(true);

    plainTextEdit->setStyleSheet(QString::fromUtf8("QAbstractScrollArea {"
                                                   " background-color: black;"
                                                   " color:green;"
                                                   "}"));

    layout->addWidget(plainTextEdit);

    for (LoggingInterface *output : PluginManager::objects<LoggingInterface>())
        registerOutput(output);

    connect(
        PluginManager::instance(), &PluginManager::objectAdded, this, &ConsoleDock::onObjectAdded);

    setWidget(widget);
}

ConsoleDock::~ConsoleDock()
{
}

void ConsoleDock::appendInfo(const QString &str)
{
    plainTextEdit->appendHtml(QLatin1String("<pre>") + str + QLatin1String("</pre>"));
}

void ConsoleDock::appendError(const QString &str)
{
    plainTextEdit->appendHtml(QLatin1String("<pre style='color:red'>") + str +
                              QLatin1String("</pre>"));
}

void ConsoleDock::onObjectAdded(QObject *object)
{
    if (LoggingInterface *output = qobject_cast<LoggingInterface *>(object))
        registerOutput(output);
}

void ConsoleDock::registerOutput(LoggingInterface *output)
{
    connect(output, &LoggingInterface::info, this, &ConsoleDock::appendInfo);
    connect(output, &LoggingInterface::error, this, &ConsoleDock::appendError);
}

} // namespace Internal
} // namespace Tiled
