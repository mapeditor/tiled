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

#include <QRegion>
#include <QSet>
#include <QString>
#include <QTimer>
#include <QVector>

class QFileSystemWatcher;
class QObject;

namespace Tiled {

class Layer;
class Map;
class TileLayer;
class Tileset;

namespace Internal {

class AutoMapper;
class MapDocument;

/**
 * This class is a superior class to the AutoMapper and AutoMapperWrapper class.
 * It uses these classes to do the whole automapping process.
 */
class AutomappingManager: public QObject
{
    Q_OBJECT

public:
    /**
     * Requests the AutomaticMapping manager. When the manager doesn't exist
     * yet, it will be created.
     */
    static AutomappingManager *instance();

    /**
     * Deletes the AutomaticMapping manager instance, when it exists.
     */
    static void deleteInstance();

    /**
     * This triggers an automapping on the whole current map document.
     */
    void automap();

    void setMapDocument(MapDocument *mapDocument);

    QString errorString() const { return mError; }

public slots:
    /**
     * This sets up new AutoMapperWrappers, which trigger the automapping.
     * The region 'where' describes where only the automapping takes place.
     * This is a signal so it can directly be connected to the regionEdited
     * signal of map documents.
     */
    void automap(QRegion where, Layer *layer);

private slots:
    /**
     * connected to the QFileWatcher, which monitors all rules files for changes
     */
    void fileChanged(const QString &path);

    /**
     * This is connected to the timer, which fires once after the files changed.
     */
    void fileChangedTimeout();

private:
    Q_DISABLE_COPY(AutomappingManager)

    /**
     * Constructor. Only used by the AutomaticMapping manager itself.
     */
    AutomappingManager(QObject *parent);

    ~AutomappingManager();

    static AutomappingManager *mInstance;

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
     * The all used rulefiles are monitored by this object, so in case of
     * external changes it will automatically reloaded.
     */
    QFileSystemWatcher *mWatcher;

    /**
     * All external changed files will be put in here, so these will be loaded
     * altogether when the timer expires.
     */
    QSet<QString> mChangedFiles;

    /**
     * This timer is started when the first file was modified. The files are
     * actually reloaded after this timer expires, so just in case the files
     * get modified within short time delays (some editors do so), it will
     * wait until all is over an reload everything at the timeout.
     */
    QTimer mChangedFilesTimer;

    QString mError;

    /**
     * This stores the name of the layer, which is used in the working map to
     * setup the automapper.
     * Until this variable was introduced it was called "set" (hardcoded)
     */
    QString mSetLayer;
};

} // namespace Internal
} // namespace Tiled

#endif // AUTOMAPPINGMANAGER_H
