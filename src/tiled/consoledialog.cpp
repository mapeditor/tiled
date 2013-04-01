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

#include "consoledialog.h"
#include "ui_consoledialog.h"
#include "pluginmanager.h"

#include <QMessageBox>
using namespace Tiled;
using namespace Tiled::Internal;

ConsoleDialog::ConsoleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConsoleDialog)
{
    ui->setupUi(this);

    ui->plainTextEdit->setStyleSheet(QString::fromUtf8(
                            "QAbstractScrollArea {"
                            " background-color: black;"
                            " color:green;"
                            "}"
                            ));

    PluginManager *pm = PluginManager::instance();

    foreach (ConsoleInterface *plg, pm->interfaces<ConsoleInterface>()) {

        connect(pm->plugin(plg)->instance, SIGNAL(info(QString)),
                this, SLOT(appendInfo(QString)));

        connect(pm->plugin(plg)->instance, SIGNAL(error(QString)),
                this, SLOT(appendError(QString)));

    }
}

void ConsoleDialog::appendInfo(QString str)
{
    ui->plainTextEdit->appendHtml(str
                    .prepend(QString::fromUtf8("<pre>"))
                    .append(QString::fromUtf8("</pre>")));
}

void ConsoleDialog::appendError(QString str)
{
    ui->plainTextEdit->appendHtml(str
                    .prepend(QString::fromUtf8("<pre style='color:red'>"))
                    .append(QString::fromUtf8("</pre>")));
}

ConsoleDialog::~ConsoleDialog()
{
    delete ui;
}
