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

#include <QDesktopServices>

using namespace Tiled::Internal;

PatreonDialog::PatreonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PatreonDialog)
{
    ui->setupUi(this);
    ui->maybeLaterButton->setVisible(false);

    connect(ui->gotoPatreon, SIGNAL(clicked()), SLOT(openPatreonPage()));
    connect(ui->alreadyPatron, SIGNAL(clicked()), SLOT(togglePatreonStatus()));

    Preferences *prefs = Preferences::instance();
    connect(prefs, SIGNAL(isPatronChanged()), SLOT(updatePatreonStatus()));

    updatePatreonStatus();
}

PatreonDialog::~PatreonDialog()
{
    delete ui;
}

void PatreonDialog::openPatreonPage()
{
    QDesktopServices::openUrl(QUrl(QLatin1String("https://www.patreon.com/bjorn")));
}

void PatreonDialog::togglePatreonStatus()
{
    Preferences *prefs = Preferences::instance();
    prefs->setPatron(!prefs->isPatron());
    setFocus();
}

void PatreonDialog::updatePatreonStatus()
{
    if (Preferences::instance()->isPatron()) {
        ui->textBrowser->setHtml(tr(
            "<html><head/><body>\n"
            "<h3>Thank you for support!</h3>\n"
            "<p>Your support as a patron makes a big difference to me as the "
            "main developer and maintainer of Tiled. It allows me to spend "
            "less time working for money elsewhere and spend more time working "
            "on Tiled instead.</p>\n"
            "<p>Keep an eye out for exclusive updates in the Activity feed on "
            "my Patreon page to find out what I've been up to in the time I "
            "could spend on Tiled thanks to your support!</p>\n"
            "<p><i>Thorbj&oslash;rn Lindeijer</i></p></body></html>"));

        ui->alreadyPatron->setText(tr("I'm no longer a patron"));
    } else {
        ui->textBrowser->setHtml(tr(
            "<html><head/><body>\n"
            "<h3>With your help I can continue to improve Tiled!</h3>\n"
            "<p>Please consider supporting me as a patron. Your support would "
            "make a big difference to me, the main developer and maintainer of "
            "Tiled. I could spend less time working for money elsewhere and "
            "spend more time working on Tiled instead.</p>\n"
            "<p>Every little bit helps. Tiled has a lot of users and if each "
            "would contribute a small donation each month I will have time to "
            "make sure Tiled keeps getting better.</p>\n"
            "<p><i>Thorbj&oslash;rn Lindeijer</i></p></body></html>"));

        ui->alreadyPatron->setText(tr("I'm already a patron!"));
    }
}
