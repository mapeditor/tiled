/*
 * objectgrouppropertiesdialog.h
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

#ifndef OBJECTGROUPPROPERTIESDIALOG_H
#define OBJECTGROUPPROPERTIESDIALOG_H

#include "propertiesdialog.h"

namespace Tiled {

class ObjectGroup;

namespace Internal {

class MapDocument;
class ColorButton;

class ObjectGroupPropertiesDialog : public PropertiesDialog
{
    Q_OBJECT

public:
    ObjectGroupPropertiesDialog(MapDocument *mapDocument,
                                ObjectGroup *objectGroup,
                                QWidget *parent = 0);

    void accept();

private:
    MapDocument *mMapDocument;
    ObjectGroup *mObjectGroup;
    ColorButton *mColorButton;
};

} // namespace Internal
} // namespace Tiled

#endif // OBJECTGROUPPROPERTIESDIALOG_H
