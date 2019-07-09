/*
 * newversiondialog.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "newversiondialog.h"
#include "ui_newversiondialog.h"

#include "tiledproxystyle.h"
#include "utils.h"

#include <QDesktopServices>
#include <QGuiApplication>

namespace Tiled {

NewVersionDialog::NewVersionDialog(const NewVersionChecker::VersionInfo &versionInfo,
                                   QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewVersionDialog)
    , mVersionInfo(versionInfo)
{
    ui->setupUi(this);

    if (auto *style = qobject_cast<TiledProxyStyle*>(QApplication::style()))
        if (style->isDark())
            ui->logo->setPixmap(QPixmap(QString::fromUtf8(":/images/about-tiled-logo-white.png")));

    QSize logoSize = Utils::dpiScaled(QSize(210, 114));
    ui->logo->setFixedSize(logoSize);
    ui->label->setFixedWidth(ui->logo->minimumWidth());
    ui->label->setText(tr("<p><b>%1 %2</b> is available!<br/><br/>Current version is %1 %3.</p>")
                       .arg(QGuiApplication::applicationDisplayName(),
                            versionInfo.version,
                            QGuiApplication::applicationVersion()));

    ui->downloadButton->setVisible(!mVersionInfo.downloadUrl.isEmpty());
    connect(ui->downloadButton, &QPushButton::clicked, this, [this] {
        QDesktopServices::openUrl(mVersionInfo.downloadUrl);
        close();
    });

    ui->releaseNotesButton->setVisible(!mVersionInfo.releaseNotesUrl.isEmpty());
    connect(ui->releaseNotesButton, &QPushButton::clicked, this, [this] {
        QDesktopServices::openUrl(mVersionInfo.releaseNotesUrl);
        close();
    });
}

NewVersionDialog::~NewVersionDialog()
{
    delete ui;
}

} // namespace Tiled
