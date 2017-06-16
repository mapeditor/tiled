/*
 * objecttemplatemodel.h
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Mohamed Thabet <thabetx@gmail.com>
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

#pragma once

#include "templategroupdocument.h"

#include <QAbstractItemModel>

namespace Tiled {
namespace Internal {

class ObjectTemplateModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ObjectTemplateModel(QObject *parent = nullptr);

    const TemplateDocuments templateDocuments() const;
    void setTemplateDocuments(const TemplateDocuments &templateDocuments);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool addNewDocument(QString fileName, QString *error);

private:
    TemplateDocuments mTemplateDocuments;
    ObjectTemplate *toObjectTemplate(const QModelIndex &index) const;
    TemplateGroup *toTemplateGroup(const QModelIndex &index) const;
};

inline void ObjectTemplateModel::setTemplateDocuments(const TemplateDocuments &templateDocuments)
{ mTemplateDocuments = templateDocuments; }

inline const TemplateDocuments ObjectTemplateModel::templateDocuments() const
{ return mTemplateDocuments; }

} // namespace Internal
} // namespace Tiled
