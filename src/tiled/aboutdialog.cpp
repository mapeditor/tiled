/*
 * aboutdialog.cpp
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Dennis Honeyman <arcticuno@gmail.com>
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

#include "aboutdialog.h"

#include "tiledproxystyle.h"
#include "utils.h"

#include <QApplication>
#include <QDesktopServices>

using namespace Tiled::Internal;

AboutDialog::AboutDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    logo->setMinimumWidth(Utils::dpiScaled(logo->minimumWidth()));
    textBrowser->setMinimumHeight(Utils::dpiScaled(160));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    const QString html = QCoreApplication::translate(
            "AboutDialog",
            "<p align=\"center\"><font size=\"+2\"><b>Tiled Map Editor</b></font><br><i>Version %1</i></p>\n"
            "<p align=\"center\">Copyright 2008-2016 Thorbj&oslash;rn Lindeijer<br>(see the AUTHORS file for a full list of contributors)</p>\n"
            "<p align=\"center\">You may modify and redistribute this program under the terms of the GPL (version 2 or later). "
            "A copy of the GPL is contained in the 'COPYING' file distributed with Tiled.</p>\n"
            "<p align=\"center\"><a href=\"http://www.mapeditor.org/\">http://www.mapeditor.org/</a></p>\n")
            .arg(QApplication::applicationVersion());

    textBrowser->setHtml(html);

    if (auto *style = qobject_cast<TiledProxyStyle*>(QApplication::style()))
        if (style->isDark())
            logo->setPixmap(QPixmap(QString::fromUtf8(":/images/about-tiled-logo-white.png")));

    connect(donateButton, SIGNAL(clicked()), SLOT(donate()));
}

void AboutDialog::donate()
{
    QDesktopServices::openUrl(QUrl(QLatin1String("http://www.mapeditor.org/donate")));
}
