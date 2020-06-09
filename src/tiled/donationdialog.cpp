/*
 * donationdialog.cpp
 * Copyright 2015-2019, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "donationdialog.h"
#include "ui_donationdialog.h"

#include "preferences.h"
#include "utils.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>
#include <QMenu>

using namespace Tiled;

DonationDialog::DonationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DonationDialog)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif

    ui->setupUi(this);

    resize(Utils::dpiScaled(size()));

    const QDate today(QDate::currentDate());

    auto laterMenu = new QMenu(this);
    laterMenu->addAction(tr("Remind me next week"))->setData(today.addDays(7));
    laterMenu->addAction(tr("Remind me next month"))->setData(today.addMonths(1));
    laterMenu->addAction(tr("Don't remind me"))->setData(QDate());
    ui->maybeLaterButton->setMenu(laterMenu);

    connect(ui->visitDonatePage, &QPushButton::clicked, this, &DonationDialog::openDonationPage);
    connect(ui->alreadyDonating, &QPushButton::clicked, this, &DonationDialog::sayThanks);
    connect(laterMenu, &QMenu::triggered, this, &DonationDialog::maybeLater);
}

DonationDialog::~DonationDialog()
{
    delete ui;
}

void DonationDialog::openDonationPage()
{
    QDesktopServices::openUrl(QUrl(QLatin1String("https://www.mapeditor.org/donate")));
}

void DonationDialog::sayThanks()
{
    Preferences *prefs = Preferences::instance();
    prefs->setPatron(true);

    QMessageBox box(QMessageBox::NoIcon, tr("Thanks!"),
                    tr("Thanks a lot for your support! With your help Tiled will keep getting better."),
                    QMessageBox::Close, this);
    box.exec();

    close();
}

void DonationDialog::maybeLater(QAction *action)
{
    const QDate date = action->data().toDate();
    Preferences::instance()->setDonationDialogReminder(date);
    close();
}
