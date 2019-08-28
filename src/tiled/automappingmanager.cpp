/*
 * automappingmanager.cpp
 * Copyright 2010-2011, Stefan Beller, stefanbeller@googlemail.com
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

#include "automappingmanager.h"

#include "automapperwrapper.h"
#include "logginginterface.h"
#include "map.h"
#include "mapdocument.h"
#include "preferences.h"
#include "tilelayer.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTextStream>

#include "qtcompat_p.h"

#include <memory>

using namespace Tiled;

AutomappingManager::AutomappingManager(QObject *parent)
    : QObject(parent)
    , mMapDocument(nullptr)
    , mLoaded(false)
{
    connect(&mWatcher, &QFileSystemWatcher::fileChanged,
            this, &AutomappingManager::onFileChanged);
}

AutomappingManager::~AutomappingManager()
{
    cleanUp();
}

void AutomappingManager::autoMap()
{
    if (!mMapDocument)
        return;

    Map *map = mMapDocument->map();
    QRegion region = mMapDocument->selectedArea();

    if (region.isEmpty()) {
        if (map->infinite()) {
            LayerIterator iterator(map);

            QRect bounds;
            while (Layer *layer = iterator.next()) {
                if (TileLayer *tileLayer = dynamic_cast<TileLayer*>(layer))
                    bounds = bounds.united(tileLayer->bounds());
            }
            region = bounds;
        } else {
            int w = map->width();
            int h = map->height();
            region = QRect(0, 0, w, h);
        }
    }

    autoMapInternal(region, nullptr);
}

void AutomappingManager::onRegionEdited(const QRegion &where, Layer *touchedLayer)
{
    if (Preferences::instance()->automappingDrawing())
        autoMapInternal(where, touchedLayer);
}

void AutomappingManager::autoMapInternal(const QRegion &where,
                                         Layer *touchedLayer)
{
    mError.clear();
    mWarning.clear();
    if (!mMapDocument)
        return;

    const bool automatic = touchedLayer != nullptr;

    if (!mLoaded) {
        const QString rulesFile = rulesFileName();
        if (loadFile(rulesFile)) {
            mLoaded = true;
        } else {
            emit errorsOccurred(automatic);
            return;
        }
    }

    QVector<AutoMapper*> passedAutoMappers;
    if (touchedLayer) {
        for (AutoMapper *a : qAsConst(mAutoMappers)) {
            if (a->ruleLayerNameUsed(touchedLayer->name()))
                passedAutoMappers.append(a);
        }
    } else {
        passedAutoMappers = mAutoMappers;
    }
    if (!passedAutoMappers.isEmpty()) {
        // use a copy of the region, so each automapper can manipulate it and the
        // following automappers do see the impact
        QRegion region(where);

        QUndoStack *undoStack = mMapDocument->undoStack();
        undoStack->beginMacro(tr("Apply AutoMap rules"));
        AutoMapperWrapper *aw = new AutoMapperWrapper(mMapDocument, passedAutoMappers, &region);
        undoStack->push(aw);
        undoStack->endMacro();
    }
    for (AutoMapper *automapper : qAsConst(mAutoMappers)) {
        mWarning += automapper->warningString();
        mError += automapper->errorString();
    }

    if (!mWarning.isEmpty())
        emit warningsOccurred(automatic);

    if (!mError.isEmpty())
        emit errorsOccurred(automatic);
}

bool AutomappingManager::loadFile(const QString &filePath)
{
    bool ret = true;
    const QDir absPath = QFileInfo(filePath).dir();
    QFile rulesFile(filePath);

    if (!rulesFile.exists()) {
        QString error = tr("No rules file found at '%1'").arg(filePath);
        ERROR(error);

        mError += error;
        mError += QLatin1Char('\n');
        return false;
    }
    if (!rulesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString error = tr("Error opening rules file '%1'").arg(filePath);
        ERROR(error);

        mError += error;
        mError += QLatin1Char('\n');
        return false;
    }

    mWatcher.addPath(filePath);

    QTextStream in(&rulesFile);
    QString line = in.readLine();

    for (; !line.isNull(); line = in.readLine()) {
        QString rulePath = line.trimmed();
        if (rulePath.isEmpty()
                || rulePath.startsWith(QLatin1Char('#'))
                || rulePath.startsWith(QLatin1String("//")))
            continue;

        QFileInfo rulePathInfo(rulePath);

        if (rulePathInfo.isRelative()) {
            rulePath = absPath.filePath(rulePath);
            rulePathInfo.setFile(rulePath);
        }

        if (!rulePathInfo.exists()) {
            QString error = tr("File not found: '%1' (referenced by '%2')")
                    .arg(rulePath, filePath);
            ERROR(error);

            mError += error;
            mError += QLatin1Char('\n');
            ret = false;
            continue;
        }
        if (rulePath.endsWith(QLatin1String(".tmx"), Qt::CaseInsensitive)) {
            QString errorString;
            std::unique_ptr<Map> rules { readMap(rulePath, &errorString) };

            if (!rules) {
                QString error = tr("Opening rules map '%1' failed: %2")
                        .arg(rulePath, errorString);
                ERROR(error);

                mError += error;
                mError += QLatin1Char('\n');
                ret = false;
                continue;
            }

            std::unique_ptr<AutoMapper> autoMapper { new AutoMapper(mMapDocument, std::move(rules), rulePath) };

            mWarning += autoMapper->warningString();
            const QString error = autoMapper->errorString();
            if (error.isEmpty()) {
                mAutoMappers.append(autoMapper.release());
                mWatcher.addPath(rulePath);
            } else {
                mError += error;
            }
        }
        if (rulePath.endsWith(QLatin1String(".txt"), Qt::CaseInsensitive)) {
            if (!loadFile(rulePath))
                ret = false;
        }
    }
    return ret;
}

void AutomappingManager::setMapDocument(MapDocument *mapDocument)
{
    QString oldRules;
    if (mMapDocument) {
        oldRules = rulesFileName();
        mMapDocument->disconnect(this);
    }

    mMapDocument = mapDocument;

    QString newRules;
    if (mMapDocument) {
        connect(mMapDocument, &MapDocument::regionEdited,
                this, &AutomappingManager::onRegionEdited);
        newRules = rulesFileName();
    }

    if (newRules != oldRules)
        cleanUp();
}

void AutomappingManager::cleanUp()
{
    qDeleteAll(mAutoMappers);
    mAutoMappers.clear();
    mLoaded = false;
    if (!mWatcher.files().isEmpty())
        mWatcher.removePaths(mWatcher.files());
}

void AutomappingManager::onFileChanged()
{
    cleanUp();
}

QString AutomappingManager::rulesFileName() const
{
    const QString mapPath = QFileInfo(mMapDocument->fileName()).path();
    const QString rulesFileName = mapPath + QLatin1String("/rules.txt");
    return rulesFileName;
}
