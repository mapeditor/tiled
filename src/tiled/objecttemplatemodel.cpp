/*
 * objecttemplatemodel.cpp
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

#include "objecttemplatemodel.h"

#include "objecttemplate.h"
#include "objecttemplateformat.h"
#include "pluginmanager.h"
#include "templatemanager.h"
#include "utils.h"

#include <QMimeData>

namespace Tiled {
namespace Internal {

ObjectTemplateModel::ObjectTemplateModel(QObject *parent):
    QFileSystemModel(parent)
{
    QStringList nameFilters;

    for (ObjectTemplateFormat *format : PluginManager::objects<ObjectTemplateFormat>()) {
        if (!(format->capabilities() & FileFormat::Read))
            continue;

        const QString filter = format->nameFilter();
        nameFilters.append(Utils::cleanFilterList(filter));
    }

    setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    setNameFilters(nameFilters);
    setNameFilterDisables(false); // hide filtered files
}

ObjectTemplateModel::~ObjectTemplateModel()
{
}

int ObjectTemplateModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;           // show only the file name column
}

ObjectTemplate *ObjectTemplateModel::toObjectTemplate(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    QFileInfo info = fileInfo(index);
    if (info.isDir())
        return nullptr;

    ObjectTemplate *objectTemplate = TemplateManager::instance()->loadObjectTemplate(info.filePath());
    return objectTemplate->object() ? objectTemplate : nullptr;
}

Qt::ItemFlags ObjectTemplateModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QFileSystemModel::flags(index);

    if (!toObjectTemplate(index))   // only valid templates can be dragged
        flags &= ~Qt::ItemIsDragEnabled;

    return flags;
}

QStringList ObjectTemplateModel::mimeTypes() const
{
    return QStringList {
        QLatin1String(TEMPLATES_MIMETYPE)
    };
}

QMimeData *ObjectTemplateModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty())
        return nullptr;

    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes)
        if (ObjectTemplate *objectTemplate = toObjectTemplate(index))
            stream << objectTemplate->fileName();

    mimeData->setData(QLatin1String(TEMPLATES_MIMETYPE), encodedData);
    return mimeData;
}

} // namespace Internal
} // namespace Tiled
