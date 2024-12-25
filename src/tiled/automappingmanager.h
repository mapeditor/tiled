/*
 * automappingmanager.h
 * Copyright 2010-2012, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2013-2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "session.h"

#include <QFileSystemWatcher>
#include <QObject>
#include <QRegion>
#include <QRegularExpression>
#include <QString>

#include <memory>
#include <vector>

namespace Tiled {

class TileLayer;

class AutoMapper;
class MapDocument;

/**
 * This class is a superior class to the AutoMapper and AutoMapperWrapper class.
 * It uses these classes to do the whole automapping process.
 */
class AutomappingManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AutomappingManager)

public:
    AutomappingManager(QObject *parent = nullptr);
    ~AutomappingManager() override;

    void setMapDocument(MapDocument *mapDocument, const QString &rulesFile = QString());

    void refreshRulesFile(const QString &ruleFileOverride = QString());

    QString errorString() const { return mError; }
    QString warningString() const { return mWarning; }

    /**
     * This triggers an automapping on the current map document. Starts with
     * the currently selected area, or the entire map if there is no selection.
     */
    void autoMap();
    void autoMapRegion(const QRegion &region);

    static SessionOption<bool> automappingWhileDrawing;

signals:
    /**
     * This signal is emitted after automapping was done and an error occurred.
     */
    void errorsOccurred(bool automatic);

    /**
     * This signal is emitted after automapping was done and a warning occurred.
     */
    void warningsOccurred(bool automatic);

private:
    void onRegionEdited(const QRegion &where, TileLayer *touchedLayer);
    void onMapFileNameChanged();
    void onFileChanged();

    bool loadFile(const QString &filePath);
    bool loadRulesFile(const QString &filePath);
    bool loadRuleMap(const QString &filePath);

    /**
     * Applies automapping to the region \a where.
     *
     * If a \a touchedLayer is given, only those AutoMappers will be used which
     * have a rule layer matching the \a touchedLayer.
     */
    void autoMapInternal(const QRegion &where, const TileLayer *touchedLayer);

    /**
     * deletes all its data structures
     */
    void cleanUp();

    /**
     * The current map document.
     */
    MapDocument *mMapDocument = nullptr;

    /**
     * For each new file of rules a new AutoMapper is setup. In this vector we
     * can store all of the AutoMappers in order.
     */
    std::vector<std::unique_ptr<AutoMapper>> mAutoMappers;

    /**
     * This tells you if the rules for the current map document were already
     * loaded.
     */
    bool mLoaded = false;

    /**
     * Contains all errors which occurred until canceling.
     * If mError is not empty, no serious result can be expected.
     */
    QString mError;

    /**
     * Contains all strings, which try to explain unusual and unexpected
     * behavior.
     */
    QString mWarning;

    QFileSystemWatcher mWatcher;

    QString mRulesFile;
    QRegularExpression mMapNameFilter;
    bool mRulesFileOverride = false;
};

} // namespace Tiled
