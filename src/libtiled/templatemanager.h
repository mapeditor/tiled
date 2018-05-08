/*
 * templatemanager.h
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

#include "tiled_global.h"

#include <QHash>
#include <QObject>

namespace Tiled {

class ObjectTemplate;

class TILEDSHARED_EXPORT TemplateManager : public QObject
{
    Q_OBJECT

public:
    static TemplateManager *instance();
    static void deleteInstance();

    ObjectTemplate *findObjectTemplate(const QString &fileName);
    ObjectTemplate *loadObjectTemplate(const QString &fileName,
                                       QString *error = nullptr);

private:
    Q_DISABLE_COPY(TemplateManager)

    TemplateManager(QObject *parent = nullptr);
    ~TemplateManager();

    QHash<QString, ObjectTemplate*> mObjectTemplates;

    static TemplateManager *mInstance;
};

inline ObjectTemplate *TemplateManager::findObjectTemplate(const QString &fileName)
{
    return mObjectTemplates.value(fileName);
}

} // namespace Tiled::Internal
