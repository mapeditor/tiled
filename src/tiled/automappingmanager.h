/*
 * automappingmanager.h
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

#ifndef AUTOMAPPINGMANAGER_H
#define AUTOMAPPINGMANAGER_H

#include <QObject>
#include <QRegion>
#include <QString>
#include <QVector>

namespace Tiled {

class Layer;

namespace Internal {

class AutoMapper;
class MapDocument;

/**
 * This class is a superior class to the AutoMapper and AutoMapperWrapper class.
 * It uses these classes to do the whole automapping process.
 */
class AutomappingManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    AutomappingManager(QObject *parent = 0);

    ~AutomappingManager();

    void setMapDocument(MapDocument *mapDocument);

    QString errorString() const { return mError; }

    QString warningString() const { return mWarning; }

signals:
    /**
     * This signal is emited after automapping was done and an error occurred.
     */
    void errorsOccurred();

    /**
     * This signal is emited after automapping was done and a warning occurred.
     */
    void warningsOccurred();

public slots:
    /**
     * This triggers an automapping on the whole current map document.
     */
    void autoMap();

private slots:
    void autoMap(const QRegion &where, Layer *touchedLayer);

private:
    Q_DISABLE_COPY(AutomappingManager)

    /**
     * This function parses a rules file.
     * For each path which is a rule, (fileextension is tmx) an AutoMapper
     * object is setup.
     *
     * If a fileextension is txt, this file will be opened and searched for
     * rules again.
     *
     * @return if the loading was successful: return true if it suceeded.
     */
    bool loadFile(const QString &filePath);

    /**
     * Applies automapping to the Region \a where, considering only layer
     * \a touchedLayer has changed.
     * There will only those Automappers be used which have a rule layer
     * touching the \a touchedLayer
     * If layer is 0, all Automappers are used.
     */
    void autoMapInternal(const QRegion &where, Layer *touchedLayer);

    /**
     * deletes all its data structures
     */
    void cleanUp();

    /**
     * The current map document.
     */
    MapDocument *mMapDocument;

    /**
     * For each new file of rules a new AutoMapper is setup. In this vector we
     * can store all of the AutoMappers in order.
     */
    QVector<AutoMapper*> mAutoMappers;

    /**
     * This tells you if the rules for the current map document were already
     * loaded.
     */
    bool mLoaded;

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
};

} // namespace Internal
} // namespace Tiled

#endif // AUTOMAPPINGMANAGER_H
