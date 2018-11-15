/*
 * objecttemplateformat.cpp
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

#include "objecttemplateformat.h"

#include "mapreader.h"

namespace Tiled {

ObjectTemplate *readObjectTemplate(const QString &fileName, QString *error)
{
    if (ObjectTemplateFormat *format = findSupportingTemplateFormat(fileName)) {
        ObjectTemplate *objectTemplate = format->read(fileName);

        if (error) {
            if (!objectTemplate)
                *error = format->errorString();
            else
                *error = QString();
        }

        if (objectTemplate)
            objectTemplate->setFormat(format);

        return objectTemplate;
    }

    return nullptr;
}

ObjectTemplateFormat *findSupportingTemplateFormat(const QString &fileName)
{
    for (ObjectTemplateFormat *format : PluginManager::objects<ObjectTemplateFormat>())
        if (format->supportsFile(fileName))
            return format;
    return nullptr;
}

} // namespace Tiled
