/*
 * rtbvalidatormodel.h
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

#ifndef RTBVALIDATORMODEL_H
#define RTBVALIDATORMODEL_H

#include "rtbvalidatorrule.h"

#include <QAbstractListModel>
#include <QIcon>


namespace Tiled {
namespace Internal {

class MapDocument;

class RTBValidatorModel : public QAbstractListModel
{
    Q_OBJECT

public:
    RTBValidatorModel(QObject *parent = 0);

    /**
     * Returns the data stored under the given <i>role</i> for the item
     * referred to by the <i>index</i>.
     */
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const;

    /**
     * Makes sure the items are checkable and names editable.
     */
    Qt::ItemFlags flags(const QModelIndex &index) const;

    /**
     * Returns the number of rows.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /**
     * Returns the headers for the table.
     */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    /**
     * Returns the map document associated with this model.
     */
    MapDocument *mapDocument() const { return mMapDocument; }

    /**
     * Sets the map document associated with this model.
     */
    void setMapDocument(MapDocument *mapDocument);

    /**
     * Returns the rules associated with this model.
     */
    QList<RTBValidatorRule*> rules() const { return mRules; }

    /**
     * Append a rule with this model.
     */
    void appendRule(RTBValidatorRule *rule);

    /**
     * Clear the rule list
     */
    void clearRules();

    MapObject *findMapObject(int row);
    RTBValidatorRule *findRule(int row);

    void highlightRules();

    int maxMessageLenght() { return mMaxMessageLenght; }
    void setMaxMessageLenght(int lenght)
    {
        mMaxMessageLenght = lenght;
        emit maxLenghtChanged(mMaxMessageLenght);
    }

signals:
    void highlightToolbarAction(int id);
    void maxLenghtChanged(int lenght);

private:
    MapDocument *mMapDocument;
    QList<RTBValidatorRule *> mRules;

    int mMaxMessageLenght;

};

} // namespace Internal
} // namespace Tiled

#endif // RTBVALIDATORMODEL_H
