/*
 * rtbvalidatordock.h
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#ifndef RTBVALIDATORDOCK_H
#define RTBVALIDATORDOCK_H

#include <QDockWidget>
#include <QTreeView>

namespace Tiled {

class MapObject;

namespace Internal {

class MapDocument;
class ValidatorView;
class RTBValidatorRule;

class RTBValidatorDock : public QDockWidget
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    RTBValidatorDock(QWidget *parent = 0);

    ~RTBValidatorDock();

    void setMapDocument(MapDocument *mapDocument);    

signals:
    void validatorItemClicked(MapObject*);
    void validatorItemClicked(int);

private slots:
    void selectMapObject(MapObject*);
    void activateToolbarAction(int);

private:
    void retranslateUi();

    MapDocument *mMapDocument;
    ValidatorView *mValidatorView;

};

/**
 *
 */
class ValidatorView : public QTreeView
{
    Q_OBJECT

public:
    explicit ValidatorView(QWidget *parent = 0);

    void setMapDocument(MapDocument *mapDocument);
    QSize sizeHint() const;

signals:
    void validatorItemClicked(MapObject*);
    void validatorItemClicked(int);

private slots:
    void indexPressed(const QModelIndex &index);
    void changeMinSize(int lenght);

private:
    MapDocument *mMapDocument;
};

} // namespace Internal
} // namespace Tiled

#endif // RTBVALIDATORDOCK_H
