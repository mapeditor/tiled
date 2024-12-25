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

#include <memory>

#include "objecttemplate.h"
#include "objecttemplateformat.h"
#include "logginginterface.h"

#include <QFile>
#include <QFileInfo>

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
    : QObject(parent),
      mWatcher(new FileSystemWatcher(this))
{
    connect(mWatcher, &FileSystemWatcher::pathsChanged,
            this, &TemplateManager::pathsChanged);
}

TemplateManager::~TemplateManager()
{
    qDeleteAll(mObjectTemplates);
}

ObjectTemplate *TemplateManager::loadObjectTemplate(const QString &fileName, QString *error)
{
    ObjectTemplate *objectTemplate = findObjectTemplate(fileName);

    if (!objectTemplate) {
        auto newTemplate = readObjectTemplate(fileName, error);

        // This instance will not have an object. It is used to detect broken
        // template references.
        if (!newTemplate)
            newTemplate = std::make_unique<ObjectTemplate>(fileName);

        // Watch the file, regardless of whether the parse was successful.
        mWatcher->addPath(fileName);

        objectTemplate = newTemplate.get();
        mObjectTemplates.insert(fileName, newTemplate.release());
    }

    return objectTemplate;
}

void TemplateManager::pathsChanged(const QStringList &paths)
{
    for (const QString &fileName : paths) {
        ObjectTemplate *objectTemplate = findObjectTemplate(fileName);

        // Most likely the file was removed.
        if (!objectTemplate)
            continue;

        // Check whether we were the ones saving this file.
        if (objectTemplate->lastSaved() == QFileInfo(fileName).lastModified())
            continue;

        auto newTemplate = readObjectTemplate(fileName);
        if (newTemplate) {
            objectTemplate->setObject(newTemplate->object());
            objectTemplate->setFormat(newTemplate->format());
            emit objectTemplateChanged(objectTemplate);
        } else if (objectTemplate->object()) {  // only report error if it had loaded fine before
            ERROR(tr("Unable to reload template file: %1").arg(fileName));
        }
    }
}

#include "moc_templatemanager.cpp"
