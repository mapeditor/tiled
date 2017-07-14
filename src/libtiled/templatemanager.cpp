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
#include "templategroupformat.h"

using namespace Tiled;

TemplateManager::TemplateManager(QObject *parent)
    : QObject(parent)
{
}

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

TemplateGroup *TemplateManager::findTemplateGroup(const QString &fileName)
{
    for (auto *group : mTemplateGroups) {
        if (group->fileName() == fileName)
            return group;
    }

    return nullptr;
}

TemplateGroup *TemplateManager::loadTemplateGroup(const QString &fileName, QString *error)
{
    TemplateGroup *templateGroup = findTemplateGroup(fileName);

    if (!templateGroup) {
        templateGroup = readTemplateGroup(fileName, error);
        // This templateGroup is specifically read for a certian map and is not part of the templates model
        if (templateGroup)
            templateGroup->setEmbedded(false);
    }

    return templateGroup;
}

const ObjectTemplate *TemplateManager::findTemplate(const QString &fileName, unsigned templateId)
{
    TemplateGroup *group = findTemplateGroup(fileName);
    if (!group)
        return nullptr;
    return group->findTemplate(templateId);;
}

