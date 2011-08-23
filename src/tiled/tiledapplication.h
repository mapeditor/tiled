/*
 * tiledapplication.h
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef TILEDAPPLICATION_H
#define TILEDAPPLICATION_H

#include <QApplication>

namespace Tiled {
namespace Internal {

/**
 * Custom QApplication subclass which handles the QFileOpenEvent, in order
 * to be able to open files appropriately on MacOS X.
 */
class TiledApplication : public QApplication
{
    Q_OBJECT

public:
    TiledApplication(int &argc, char **argv);

protected:
    bool event(QEvent *);

signals:
    void fileOpenRequest(const QString &file);
};

} // namespace Internal
} // namespace Tiled

#endif // TILEDAPPLICATION_H
