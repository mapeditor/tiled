/*
 * donationpopup.cpp
 * Copyright 2015-2021, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "donationpopup.h"

#include "preferences.h"
#include "utils.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>

using namespace Tiled;

DonationPopup::DonationPopup(QWidget *parent)
    : PopupWidget(parent)
{
    setTint(Qt::green);

    auto label = new QLabel(QCoreApplication::translate("DonationDialog", "Please consider supporting Tiled development with a small monthly donation."));

    auto visitDonatePage = new QPushButton(QCoreApplication::translate("DonationDialog", "&Donate ↗"));
    auto alreadyDonating = new QPushButton(QCoreApplication::translate("DonationDialog", "I'm a &supporter!"));
    auto maybeLaterButton = new QPushButton(QCoreApplication::translate("DonationDialog", "&Maybe later"));

    const QDate today(QDate::currentDate());
    auto laterMenu = new QMenu(this);
    laterMenu->addAction(QCoreApplication::translate("Tiled::DonationDialog", "Remind me next week"))->setData(today.addDays(7));
    laterMenu->addAction(QCoreApplication::translate("Tiled::DonationDialog", "Remind me next month"))->setData(today.addMonths(1));
    laterMenu->addAction(QCoreApplication::translate("Tiled::DonationDialog", "Don't remind me"))->setData(QDate());
    maybeLaterButton->setMenu(laterMenu);

    auto layout = new QHBoxLayout;
    layout->addWidget(label);
    layout->addSpacing(Utils::dpiScaled(10));
    layout->addWidget(visitDonatePage);
    layout->addWidget(alreadyDonating);
    layout->addWidget(maybeLaterButton);
    const auto margin = Utils::dpiScaled(5);
    layout->setContentsMargins(margin * 2, margin, margin, margin);
    setLayout(layout);

    connect(visitDonatePage, &QPushButton::clicked, this, &DonationPopup::openDonationPage);
    connect(alreadyDonating, &QPushButton::clicked, this, &DonationPopup::sayThanks);
    connect(laterMenu, &QMenu::triggered, this, &DonationPopup::maybeLater);
}

void DonationPopup::openDonationPage()
{
    QDesktopServices::openUrl(QUrl(QLatin1String("https://www.mapeditor.org/donate")));
}

void DonationPopup::sayThanks()
{
    Preferences::instance()->setPatron(true);

    QMessageBox(QMessageBox::NoIcon, QCoreApplication::translate("Tiled::DonationDialog", "Thanks!"),
                QCoreApplication::translate("Tiled::DonationDialog", "Thanks a lot for your support! With your help Tiled will keep getting better."),
                QMessageBox::Close, this).exec();

    close();
}

void DonationPopup::maybeLater(QAction *action)
{
    const QDate date = action->data().toDate();
    Preferences::instance()->setDonationReminder(date);
    close();
}

#include "moc_donationpopup.cpp"
