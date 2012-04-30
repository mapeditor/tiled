/*
 * objectpropertiesdialog.h
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Michael Woerister <michaelwoerister@gmail.com>
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

#ifndef OBJECTPROPERTIESDIALOG_H
#define OBJECTPROPERTIESDIALOG_H

#include "propertiesdialog.h"

class QLineEdit;

namespace Ui {
class ObjectPropertiesDialog;
}

namespace Tiled {

class MapObject;

namespace Internal {

class MapDocument;

class ObjectPropertiesDialog : public PropertiesDialog
{
    Q_OBJECT

public:
    ObjectPropertiesDialog(MapDocument *mapDocument,
                           MapObject *mapObject,
                           QWidget *parent = 0);

    ~ObjectPropertiesDialog();

    void accept();

private:
    MapDocument *mMapDocument;
    MapObject *mMapObject;

    Ui::ObjectPropertiesDialog *mUi;
};

} // namespace Internal
} // namespace Tiled

#endif // OBJECTPROPERTIESDIALOG_H
