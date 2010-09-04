/*
 * propertiesmodel.h
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef PROPERTIESMODEL_H
#define PROPERTIESMODEL_H

#include "properties.h"

#include <QAbstractItemModel>
#include <QList>
#include <QString>

namespace Tiled {
namespace Internal {

class PropertiesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    PropertiesModel(QObject *parent = 0);

    /**
     * Returns the number of rows.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /**
     * Returns the number of columns.
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    /**
     * Returns the data stored under the given <i>role</i> for the item
     * referred to by the <i>index</i>.
     */
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const;

    /**
     * Returns the item flags for the given <i>index</i>. In addition to
     * enabled and selectable, items are editable.
     */
    Qt::ItemFlags flags(const QModelIndex &index) const;

    /**
     * Sets the value of the data at the given <i>index</i>. This model
     * supports the Qt::EditRole for both columns.
     */
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);

    /**
     * Deletes the list of properties associated with the given indices.
     */
    void deleteProperties(const QModelIndexList &indices);

    /**
     * Returns the headers for the table.
     */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    /**
     * Sets the properties displayed by this model.
     */
    void setProperties(const Properties &properties);

    /**
     * Returns the, possibly edited, properties.
     */
    const Properties &properties() const;

private:
    Properties mProperties;
    QList<QString> mKeys;
};

} // namespace Internal
} // namespace Tiled

#endif // PROPERTIESMODEL_H
