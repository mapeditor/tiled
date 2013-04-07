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
#include "pluginmanager.h"

#include <QVBoxLayout>

using namespace Tiled;
using namespace Tiled::Internal;

ConsoleDock::ConsoleDock(QWidget *parent)
    : QDockWidget(parent)
{
    setObjectName(QLatin1String("ConsoleDock"));

    setWindowTitle(tr("Debug Console"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(5);

    plainTextEdit = new QPlainTextEdit;
    plainTextEdit->setReadOnly(true);

    plainTextEdit->setStyleSheet(QString::fromUtf8(
                            "QAbstractScrollArea {"
                            " background-color: black;"
                            " color:green;"
                            "}"
                            ));

    layout->addWidget(plainTextEdit);

    PluginManager *pm = PluginManager::instance();

    foreach (LoggingInterface *plg, pm->interfaces<LoggingInterface>()) {

        connect(pm->plugin(plg)->instance, SIGNAL(info(QString)),
                this, SLOT(appendInfo(QString)));

        connect(pm->plugin(plg)->instance, SIGNAL(error(QString)),
                this, SLOT(appendError(QString)));

    }

    setWidget(widget);
}

void ConsoleDock::appendInfo(QString str)
{
    plainTextEdit->appendHtml(str
                    .prepend(QString::fromUtf8("<pre>"))
                    .append(QString::fromUtf8("</pre>")));
}

void ConsoleDock::appendError(QString str)
{
    plainTextEdit->appendHtml(str
                    .prepend(QString::fromUtf8("<pre style='color:red'>"))
                    .append(QString::fromUtf8("</pre>")));
}

ConsoleDock::~ConsoleDock()
{
}
