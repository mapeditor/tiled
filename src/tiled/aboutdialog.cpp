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

#ifdef BUILD_INFO
#include "buildinfo.h"
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifndef BUILD_INFO_GIT
#define BUILD_INFO_GIT ""
#endif

#ifndef BUILD_INFO_TIME
#define BUILD_INFO_TIME ""
#endif

#define BUILD_INFO_GIT_WRAPPER QLatin1String(TOSTRING(BUILD_INFO_GIT))
#define BUILD_INFO_TIME_WRAPPER QLatin1String(TOSTRING(BUILD_INFO_TIME))

#include <QCoreApplication>

using namespace Tiled::Internal;

AboutDialog::AboutDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    const QString html = QCoreApplication::translate(
            "AboutDialog",
            "<p align=\"center\"><font size=\"+2\"><b>Tiled Map Editor</b></font><br><i>Version %1</i></p>\n"
            "<p align=\"center\">%2</p>\n"
            "<p align=\"center\">%3</p>\n"
            "<p align=\"center\">Copyright 2008-2011 Thorbj&oslash;rn Lindeijer<br>(see the AUTHORS file for a full list of contributors)</p>\n"
            "<p align=\"center\">You may modify and redistribute this program under the terms of the GPL (version 2 or later). "
            "A copy of the GPL is contained in the 'COPYING' file distributed with Tiled.</p>\n"
            "<p align=\"center\"><a href=\"http://www.mapeditor.org/\">http://www.mapeditor.org/</a></p>\n")
            .arg(QApplication::applicationVersion(), BUILD_INFO_GIT_WRAPPER, BUILD_INFO_TIME_WRAPPER);

    textBrowser->setHtml(html);
}
