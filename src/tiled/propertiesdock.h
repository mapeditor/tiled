/*
 * propertiesdock.h
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef PROPERTIESDOCK_H
#define PROPERTIESDOCK_H

#include <QDockWidget>

namespace Tiled {

class Object;

namespace Internal {

class MapDocument;
class PropertiesModel;
class PropertiesView;

class PropertiesDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit PropertiesDock(QWidget *parent = 0);

protected:
    void changeEvent(QEvent *e);

private slots:
    void mapDocumentChanged(MapDocument *mapDocument);
    void currentObjectChanged(Object *object);
    void deleteSelectedProperties();

private:
    void retranslateUi();

    MapDocument *mMapDocument;
    PropertiesModel *mPropertiesModel;
    PropertiesView *mPropertiesView;
};

} // namespace Internal
} // namespace Tiled

#endif // PROPERTIESDOCK_H
