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
}

void AutomappingManager::autoMap()
{
    if (!mMapDocument)
        return;

    QRegion region = mMapDocument->selectedArea();

    if (region.isEmpty()) {
        Map *map = mMapDocument->map();

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

void AutomappingManager::autoMapRegion(const QRegion &region)
{
    autoMapInternal(region, nullptr);
}

void AutomappingManager::onRegionEdited(const QRegion &where, Layer *touchedLayer)
{
    if (Preferences::instance()->automappingDrawing())
        autoMapInternal(where, touchedLayer);
}

void AutomappingManager::onMapFileNameChanged()
{
    if (!mRulesFileOverride)
        refreshRulesFile();
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
        if (loadFile(mRulesFile)) {
            mLoaded = true;
        } else {
            emit errorsOccurred(automatic);
            return;
        }
    }

    QVector<AutoMapper*> passedAutoMappers;
    for (auto &a : qAsConst(mAutoMappers)) {
        if (!touchedLayer || a->ruleLayerNameUsed(touchedLayer->name()))
            passedAutoMappers.append(a.get());
    }

    if (!passedAutoMappers.isEmpty()) {
        // use a copy of the region, so each automapper can manipulate it and the
        // following automappers do see the impact
        QRegion region(where);

        QUndoStack *undoStack = mMapDocument->undoStack();
        undoStack->beginMacro(tr("Apply AutoMap rules"));
        AutoMapperWrapper *aw = new AutoMapperWrapper(mMapDocument, std::move(passedAutoMappers), &region);
        undoStack->push(aw);
        undoStack->endMacro();
    }
    for (auto &automapper : qAsConst(mAutoMappers)) {
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
                mAutoMappers.push_back(std::move(autoMapper));
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

/**
 * The rules file is determind based on the map location, but can be overridden
 * by passing \a rulesFile.
 */
void AutomappingManager::setMapDocument(MapDocument *mapDocument, const QString &rulesFile)
{
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument) {
        connect(mMapDocument, &MapDocument::fileNameChanged,
                this, &AutomappingManager::onMapFileNameChanged);
        connect(mMapDocument, &MapDocument::regionEdited,
                this, &AutomappingManager::onRegionEdited);
    }

    refreshRulesFile(rulesFile);
}

void AutomappingManager::refreshRulesFile(const QString &ruleFileOverride)
{
    mRulesFileOverride = !ruleFileOverride.isEmpty();
    QString rulesFile = ruleFileOverride;

    if (rulesFile.isEmpty() && mMapDocument) {
        const QString mapPath = QFileInfo(mMapDocument->fileName()).path();
        rulesFile = mapPath + QLatin1String("/rules.txt");
    }

    if (mRulesFile != rulesFile) {
        mRulesFile = rulesFile;
        cleanUp();
    }
}

void AutomappingManager::cleanUp()
{
    mAutoMappers.clear();
    mLoaded = false;
    if (!mWatcher.files().isEmpty())
        mWatcher.removePaths(mWatcher.files());
}

void AutomappingManager::onFileChanged()
{
    cleanUp();
}
