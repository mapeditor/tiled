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
#include "map.h"
#include "mapdocument.h"
#include "tilelayer.h"
#include "tilesetmanager.h"
#include "tmxmapreader.h"

#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTextStream>

using namespace Tiled;
using namespace Tiled::Internal;

AutomappingManager *AutomappingManager::mInstance = 0;

AutomappingManager::AutomappingManager(QObject *parent)
    : QObject(parent)
    , mMapDocument(0)
    , mLoaded(false)
{
}

AutomappingManager::~AutomappingManager()
{
    cleanUp();
}

AutomappingManager *AutomappingManager::instance()
{
    if (!mInstance)
        mInstance = new AutomappingManager(0);

    return mInstance;
}

void AutomappingManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

void AutomappingManager::autoMap()
{
    if (!mMapDocument)
        return;

    Map *map = mMapDocument->map();
    int w = map->width();
    int h = map->height();
    int l = map->indexOfLayer(mSetLayer);

    Layer *passedLayer = 0;
    if (l != -1)
        passedLayer = map->layerAt(l);

    autoMap(QRect(0, 0, w, h), passedLayer);
}

void AutomappingManager::autoMap(QRegion where, Layer *l)
{
    mError.clear();
    mWarning.clear();
    if (!mMapDocument) {
        mError = tr("No map document found!") + QLatin1Char('\n');
        emit errorsOccurred();
        return;
    }

    if (!l) {
        mError = tr("No set layer found!") + QLatin1Char('\n');
        emit errorsOccurred();
        return;
    }

    if (l->name() != mSetLayer)
        return;

    if (!mLoaded) {
        const QString mapPath = QFileInfo(mMapDocument->fileName()).path();
        const QString rulesFileName = mapPath + QLatin1String("/rules.txt");
        if (loadFile(rulesFileName)) {
            mLoaded = true;
        } else {
            emit errorsOccurred();
            return;
        }
    }

    Map *map = mMapDocument->map();
    QString layer = map->layerAt(mMapDocument->currentLayerIndex())->name();

    // use a pointer to the region, so each automapper can manipulate it and the
    // following automappers do see the impact
    QRegion *passedRegion = new QRegion(where);

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->beginMacro(tr("Apply AutoMap rules"));
    AutoMapperWrapper *aw = new AutoMapperWrapper(mMapDocument, mAutoMappers, passedRegion);
    undoStack->push(aw);
    undoStack->endMacro();

    foreach (AutoMapper *automapper, mAutoMappers) {
        mWarning += automapper->warningString();
        mError += automapper->errorString();
    }

    mMapDocument->emitRegionChanged(*passedRegion);
    delete passedRegion;
    mMapDocument->setCurrentLayerIndex(map->indexOfLayer(layer));

    if (!mWarning.isEmpty())
        emit warningsOccurred();

    if (!mError.isEmpty())
        emit errorsOccurred();
}

bool AutomappingManager::loadFile(const QString &filePath)
{
    bool ret = true;
    const QString absPath = QFileInfo(filePath).path();
    QFile rulesFile(filePath);

    if (!rulesFile.exists()) {
        mError += tr("No rules file found at:\n%1").arg(filePath)
                  + QLatin1Char('\n');
        return false;
    }
    if (!rulesFile.open(QIODevice::ReadOnly)) {
        mError += tr("Error opening rules file:\n%1").arg(filePath)
                  + QLatin1Char('\n');
        return false;
    }

    QTextStream in(&rulesFile);
    QString line = in.readLine();

    for (; !line.isNull(); line = in.readLine()) {
        QString rulePath = line.trimmed();
        if (rulePath.isEmpty()
                || rulePath.startsWith(QLatin1Char('#'))
                || rulePath.startsWith(QLatin1String("//")))
            continue;

        if (QFileInfo(rulePath).isRelative())
            rulePath = absPath + QLatin1Char('/') + rulePath;

        if (!QFileInfo(rulePath).exists()) {
            mError += tr("File not found:\n%1").arg(rulePath) + QLatin1Char('\n');
            ret = false;
            continue;
        }
        if (rulePath.endsWith(QLatin1String(".tmx"), Qt::CaseInsensitive)){
            TmxMapReader mapReader;

            Map *rules = mapReader.read(rulePath);

            if (!rules) {
                mError += tr("Opening rules map failed:\n%1").arg(
                        mapReader.errorString()) + QLatin1Char('\n');
                ret = false;
                continue;
            }

            TilesetManager *tilesetManager = TilesetManager::instance();
            tilesetManager->addReferences(rules->tilesets());

            AutoMapper *autoMapper;
            autoMapper = new AutoMapper(mMapDocument, mSetLayer);

            bool loadSuccess = autoMapper->prepareLoad(rules, rulePath);
            mWarning += autoMapper->warningString();
            if (loadSuccess) {
                mAutoMappers.append(autoMapper);
            } else {
                mError += autoMapper->errorString();
                delete autoMapper;
            }
        }
        if (rulePath.endsWith(QLatin1String(".txt"), Qt::CaseInsensitive)){
            if (!loadFile(rulePath))
                ret = false;
        }
    }
    return ret;
}

void AutomappingManager::setMapDocument(MapDocument *mapDocument)
{
    cleanUp();
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument)
        connect(mMapDocument, SIGNAL(regionEdited(QRegion,Layer*)),
                this, SLOT(autoMap(QRegion,Layer*)));
    mLoaded = false;
}

void AutomappingManager::cleanUp()
{
    foreach (const AutoMapper *autoMapper, mAutoMappers) {
        delete autoMapper;
    }
    mAutoMappers.clear();
}
