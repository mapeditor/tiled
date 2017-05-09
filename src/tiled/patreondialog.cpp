/*
 * patreondialog.cpp
 * Copyright 2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "patreondialog.h"
#include "ui_patreondialog.h"

#include "preferences.h"
#include "utils.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>
#include <QMenu>

using namespace Tiled::Internal;

PatreonDialog::PatreonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PatreonDialog)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);

    resize(Utils::dpiScaled(size()));

    connect(ui->gotoPatreon, &QPushButton::clicked, this, &PatreonDialog::openPatreonPage);
    connect(ui->alreadyPatron, &QPushButton::clicked, this, &PatreonDialog::sayThanks);

    auto laterMenu = new QMenu;
    laterMenu->addAction(tr("Remind me next week"), [this]() { maybeLater(QDate::currentDate().addDays(7)); });
    laterMenu->addAction(tr("Remind me next month"), [this]() { maybeLater(QDate::currentDate().addMonths(1)); });
    laterMenu->addAction(tr("Don't remind me"), [this]() { maybeLater(QDate()); });

    ui->maybeLaterButton->setMenu(laterMenu);
}

PatreonDialog::~PatreonDialog()
{
    delete ui;
}

void PatreonDialog::openPatreonPage()
{
    QDesktopServices::openUrl(QUrl(QLatin1String("https://www.patreon.com/bjorn")));
}

void PatreonDialog::sayThanks()
{
    Preferences *prefs = Preferences::instance();
    prefs->setPatron(true);

    QMessageBox box(QMessageBox::NoIcon, tr("Thanks!"),
                    tr("Thanks a lot for your support! With your help Tiled will keep getting better."),
                    QMessageBox::Close, this);
    box.exec();

    close();
}

void PatreonDialog::maybeLater(const QDate &date)
{
    Preferences::instance()->setPatreonDialogReminder(date);
    close();
}
