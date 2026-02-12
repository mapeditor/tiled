/*
 * automappingmanager.cpp
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

#include "automappingmanager.h"

#include "automapper.h"
#include "automapperwrapper.h"
#include "logginginterface.h"
#include "map.h"
#include "mapdocument.h"
#include "project.h"
#include "projectmanager.h"
#include "tilelayer.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QScopeGuard>
#include <QScopedValueRollback>
#include <QTextStream>

using namespace Tiled;

SessionOption<bool> AutomappingManager::automappingWhileDrawing { "automapping.whileDrawing", false };

AutomappingManager::AutomappingManager(QObject *parent)
    : QObject(parent)
{
    mMapNameFilter.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

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
        const Map *map = mMapDocument->map();

        if (map->infinite()) {
            LayerIterator iterator(map);

            QRect bounds;
            while (Layer *layer = iterator.next()) {
                if (TileLayer *tileLayer = dynamic_cast<TileLayer*>(layer))
                    bounds = bounds.united(tileLayer->bounds());
            }
            region = bounds;
        } else {
            region = QRect(0, 0, map->width(), map->height());
        }
    }

    autoMapInternal(region, nullptr);
}

void AutomappingManager::autoMapRegion(const QRegion &region)
{
    autoMapInternal(region, nullptr);
}

void AutomappingManager::onRegionEdited(const QRegion &where, TileLayer *touchedLayer)
{
    if (automappingWhileDrawing)
        autoMapInternal(where, touchedLayer);
}

void AutomappingManager::onMapFileNameChanged()
{
    if (!mRulesFileOverride)
        refreshRulesFile();
}

void AutomappingManager::autoMapInternal(const QRegion &where,
                                         const TileLayer *touchedLayer)
{
    mError.clear();
    mWarning.clear();

    if (!mMapDocument)
        return;

    const bool automatic = touchedLayer != nullptr;

    // Even if no AutoMapper instance will be executed, we still want to report
    // any warnings or errors that might have been reported while interpreting
    // the rule maps.
    auto reportErrors = qScopeGuard([=] {
        if (!mWarning.isEmpty())
            emit warningsOccurred(automatic);

        if (!mError.isEmpty())
            emit errorsOccurred(automatic);
    });

    if (!mLoaded) {
        if (mRulesFile.isEmpty()) {
            mError = tr("No AutoMapping rules provided. Save the map or refer to a rule file in the project properties.");
            return;
        }

        if (!loadFile(mRulesFile))
            return;

        mLoaded = true;
    }

    // Determine the list of AutoMappers that is relevant for this map
    const QString mapFileName = QFileInfo(mMapDocument->fileName()).fileName();

    QVector<const AutoMapper*> autoMappers;
    autoMappers.reserve(mRuleMapReferences.size());

    for (auto &ruleMap : std::as_const(mRuleMapReferences)) {
        const auto &mapNameFilter = ruleMap.mapNameFilter;
        if (!mapNameFilter.isValid() || mapNameFilter.match(mapFileName).hasMatch())
            if (const AutoMapper *autoMapper = findAutoMapper(ruleMap.filePath))
                autoMappers.append(autoMapper);
    }

    if (autoMappers.isEmpty())
        return;

    // Skip this AutoMapping run if none of the loaded rule maps actually use
    // the touched layer.
    if (touchedLayer) {
        if (std::none_of(autoMappers.cbegin(),
                         autoMappers.cend(),
                         [=] (const AutoMapper *autoMapper) { return autoMapper->ruleLayerNameUsed(touchedLayer->name()); }))
            return;
    }

    AutoMapperWrapper *aw = new AutoMapperWrapper(mMapDocument, autoMappers, where, touchedLayer);
    aw->setMergeable(automatic);
    aw->setText(tr("Apply AutoMap rules"));

    mMapDocument->undoStack()->push(aw);
}

/**
 * Returns the AutoMapper instance for the given rules file, loading it if
 * necessary. Returns nullptr if the file could not be loaded.
 */
const AutoMapper *AutomappingManager::findAutoMapper(const QString &filePath)
{
    auto it = mLoadedAutoMappers.find(filePath);
    if (it != mLoadedAutoMappers.end())
        return it->second.get();

    auto autoMapper = loadRuleMap(filePath);
    if (!autoMapper)
        return nullptr;

    auto result = mLoadedAutoMappers.emplace(filePath, std::move(autoMapper));
    return result.first->second.get();
}

/**
 * This function parses a rules file or loads a rules map file.
 *
 * While parsing a rules file, any listed files with extension "txt" will also
 * be parsed as a rules file. Any listed file that loads as a map results in
 * the creation of an AutoMapper instance.
 *
 * @return whether the loading was successful
 */
bool AutomappingManager::loadFile(const QString &filePath)
{
    if (filePath.endsWith(QLatin1String(".txt"), Qt::CaseInsensitive)) {
        // Restore any potential change to the map name filter after processing
        // the included rules file.
        QScopedValueRollback<QRegularExpression> mapNameFilter(mMapNameFilter);

        return loadRulesFile(filePath);
    }

    mRuleMapReferences.append(RuleMapReference { filePath, mMapNameFilter });
    return true;
}

bool AutomappingManager::loadRulesFile(const QString &filePath)
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

    for (QString line = in.readLine(); !line.isNull(); line = in.readLine()) {
        auto trimmedLine = QStringView(line).trimmed();
        if (trimmedLine.isEmpty()
                || trimmedLine.startsWith(QLatin1Char('#'))
                || trimmedLine.startsWith(QLatin1String("//")))
            continue;

        if (trimmedLine.startsWith(QLatin1Char('[')) && trimmedLine.endsWith(QLatin1Char(']'))) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            auto filter = trimmedLine.mid(1, trimmedLine.length() - 2);
#else
            auto filter = trimmedLine.sliced(1, trimmedLine.length() - 2);
#endif
            mMapNameFilter.setPattern(QRegularExpression::wildcardToRegularExpression(filter));
            continue;
        }

        const QString rulePath = absPath.filePath(trimmedLine.toString());

        if (!QFileInfo::exists(rulePath)) {
            QString error = tr("File not found: '%1' (referenced by '%2')")
                    .arg(rulePath, filePath);
            ERROR(error);

            mError += error;
            mError += QLatin1Char('\n');
            ret = false;
            continue;
        }

        if (!loadFile(rulePath))
            ret = false;
    }

    return ret;
}

std::unique_ptr<AutoMapper> AutomappingManager::loadRuleMap(const QString &filePath)
{
    QString errorString;
    auto rulesMap = readMap(filePath, &errorString);
    if (!rulesMap) {
        QString error = tr("Opening rules map '%1' failed: %2")
                .arg(filePath, errorString);
        ERROR(error);

        mError += error;
        mError += QLatin1Char('\n');
        return {};
    }

    mWatcher.addPath(filePath);

    auto autoMapper = std::make_unique<AutoMapper>(std::move(rulesMap));

    mWarning += autoMapper->warningString();
    const QString error = autoMapper->errorString();
    if (!error.isEmpty())
        mError += error;

    return autoMapper;
}

/**
 * The rules file is determined based on the map location, or taken from the
 * current project if a "rules.txt" file does not exist alongside the map (or
 * when the map is not saved).
 *
 * Alternatively, it can can be overridden by passing a non-empty \a rulesFile.
 */
void AutomappingManager::setMapDocument(MapDocument *mapDocument, const QString &rulesFile)
{
    if (mMapDocument != mapDocument) {
        if (mMapDocument)
            mMapDocument->disconnect(this);

        mMapDocument = mapDocument;

        if (mMapDocument) {
            connect(mMapDocument, &MapDocument::fileNameChanged,
                    this, &AutomappingManager::onMapFileNameChanged);
            connect(mMapDocument, &MapDocument::regionEdited,
                    this, &AutomappingManager::onRegionEdited);
        }
    }

    refreshRulesFile(rulesFile);
}

/**
 * Needs to be called when the project rules file path is changed.
 *
 * It is called automatically when the file name of the current MapDocument
 * changes.
 */
void AutomappingManager::refreshRulesFile(const QString &ruleFileOverride)
{
    mRulesFileOverride = !ruleFileOverride.isEmpty();
    QString rulesFile = ruleFileOverride;

    if (rulesFile.isEmpty() && mMapDocument) {
        if (!mMapDocument->fileName().isEmpty()) {
            const QDir mapDir = QFileInfo(mMapDocument->fileName()).dir();
            rulesFile = mapDir.filePath(QStringLiteral("rules.txt"));
        }

        if (rulesFile.isEmpty() || !QFileInfo::exists(rulesFile)) {
            const auto &project = ProjectManager::instance()->project();
            if (!project.mAutomappingRulesFile.isEmpty())
                rulesFile = project.mAutomappingRulesFile;
        }
    }

    if (mRulesFile != rulesFile) {
        mRulesFile = rulesFile;
        cleanUp();
    }
}

void AutomappingManager::cleanUp()
{
    mRuleMapReferences.clear();
    mLoaded = false;
}

void AutomappingManager::onFileChanged(const QString &path)
{
    // Make sure the changed file will be reloaded
    mLoadedAutoMappers.erase(path);

    // File will be re-added when it is still relevant
    mWatcher.removePath(path);

    // Re-parse the rules file(s) when any of them changed
    if (path.endsWith(QLatin1String(".txt"), Qt::CaseInsensitive))
        cleanUp();
}

#include "moc_automappingmanager.cpp"
