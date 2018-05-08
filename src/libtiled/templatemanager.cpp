/*
 * templatemanager.cpp
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

#include "templatemanager.h"

#include "objecttemplate.h"
#include "objecttemplateformat.h"

using namespace Tiled;

TemplateManager *TemplateManager::mInstance;

TemplateManager *TemplateManager::instance()
{
    if (!mInstance)
        mInstance = new TemplateManager;

    return mInstance;
}

void TemplateManager::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

TemplateManager::TemplateManager(QObject *parent)
    : QObject(parent)
{
}

TemplateManager::~TemplateManager()
{
    qDeleteAll(mObjectTemplates);
}

ObjectTemplate *TemplateManager::loadObjectTemplate(const QString &fileName, QString *error)
{
    ObjectTemplate *objectTemplate = findObjectTemplate(fileName);

    if (!objectTemplate)
        objectTemplate = readObjectTemplate(fileName, error);

    // This instance will not have an object. It is used to detect broken
    // template references.
    if (!objectTemplate)
        objectTemplate = new ObjectTemplate(fileName);

    mObjectTemplates.insert(fileName, objectTemplate);

    return objectTemplate;
}
