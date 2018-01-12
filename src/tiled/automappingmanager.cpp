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
#include "tmxmapformat.h"
#include "preferences.h"

#include <QFileInfo>
#include <QScopedPointer>
#include <QTextStream>

using namespace Tiled;
using namespace Tiled::Internal;

AutomappingManager::AutomappingManager(QObject *parent)
    : QObject(parent)
    , mMapDocument(nullptr)
    , mLoaded(false)
{
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

    QRect bounds;

    if (map->infinite()) {
        LayerIterator iterator(map);

        while (Layer *layer = iterator.next()) {
            if (TileLayer *tileLayer = dynamic_cast<TileLayer*>(layer))
                bounds = bounds.united(tileLayer->bounds());
        }
    } else {
        int w = map->width();
        int h = map->height();
        bounds = QRect(0, 0, w, h);
    }

    autoMapInternal(bounds, nullptr);
}

void AutomappingManager::autoMap(const QRegion &where, Layer *touchedLayer)
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
        const QString mapPath = QFileInfo(mMapDocument->fileName()).path();
        const QString rulesFileName = mapPath + QLatin1String("/rules.txt");
        if (loadFile(rulesFileName)) {
            mLoaded = true;
        } else {
            emit errorsOccurred(automatic);
            return;
        }
    }

    QVector<AutoMapper*> passedAutoMappers;
    if (touchedLayer) {
        foreach (AutoMapper *a, mAutoMappers) {
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
    foreach (AutoMapper *automapper, mAutoMappers) {
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
    const QString absPath = QFileInfo(filePath).path();
    QFile rulesFile(filePath);

    if (!rulesFile.exists()) {
        mError += tr("No rules file found at:\n%1").arg(filePath)
                  + QLatin1Char('\n');
        return false;
    }
    if (!rulesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
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
        if (rulePath.endsWith(QLatin1String(".tmx"), Qt::CaseInsensitive)) {
            TmxMapFormat tmxFormat;

            QScopedPointer<Map> rules(tmxFormat.read(rulePath));

            if (!rules) {
                mError += tr("Opening rules map failed:\n%1").arg(
                        tmxFormat.errorString()) + QLatin1Char('\n');
                ret = false;
                continue;
            }

            AutoMapper *autoMapper = new AutoMapper(mMapDocument, rules.take(), rulePath);

            mWarning += autoMapper->warningString();
            const QString error = autoMapper->errorString();
            if (error.isEmpty()) {
                mAutoMappers.append(autoMapper);
            } else {
                mError += error;
                delete autoMapper;
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
    cleanUp();
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument) {
        connect(mMapDocument, SIGNAL(regionEdited(QRegion,Layer*)),
                this, SLOT(autoMap(QRegion,Layer*)));
    }

    mLoaded = false;
}

void AutomappingManager::cleanUp()
{
    qDeleteAll(mAutoMappers);
    mAutoMappers.clear();
}
