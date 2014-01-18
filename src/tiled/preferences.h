/*
 * preferences.h
 * Copyright 2009-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QObject>
#include <QColor>

#include "map.h"
#include "objecttypes.h"

class QSettings;

namespace Tiled {
namespace Internal {

/**
 * This class holds user preferences and provides a convenient interface to
 * access them.
 */
class Preferences : public QObject
{
    Q_OBJECT

public:
    static Preferences *instance();
    static void deleteInstance();

    bool showGrid() const { return mShowGrid; }
    bool showTileObjectOutlines() const { return mShowTileObjectOutlines; }
    bool showTileAnimations() const { return mShowTileAnimations; }
    bool snapToGrid() const { return mSnapToGrid; }
    bool snapToFineGrid() const { return mSnapToFineGrid; }
    QColor gridColor() const { return mGridColor; }
    int gridFine() const { return mGridFine; }
    qreal objectLineWidth() const { return mObjectLineWidth; }

    bool highlightCurrentLayer() const { return mHighlightCurrentLayer; }
    bool showTilesetGrid() const { return mShowTilesetGrid; }

    Map::LayerDataFormat layerDataFormat() const;
    void setLayerDataFormat(Map::LayerDataFormat layerDataFormat);

    bool dtdEnabled() const;
    void setDtdEnabled(bool enabled);

    QString language() const;
    void setLanguage(const QString &language);

    bool reloadTilesetsOnChange() const;
    void setReloadTilesetsOnChanged(bool value);

    bool useOpenGL() const { return mUseOpenGL; }
    void setUseOpenGL(bool useOpenGL);

    const ObjectTypes &objectTypes() const { return mObjectTypes; }
    void setObjectTypes(const ObjectTypes &objectTypes);

    enum FileType {
        ObjectTypesFile,
        ImageFile,
        ExportedFile
    };

    QString lastPath(FileType fileType) const;
    void setLastPath(FileType fileType, const QString &path);

    bool automappingDrawing() const { return mAutoMapDrawing; }
    void setAutomappingDrawing(bool enabled);

    QString mapsDirectory() const;
    void setMapsDirectory(const QString &path);

    /**
     * Provides access to the QSettings instance to allow storing/retrieving
     * arbitrary values. The naming style for groups and keys is CamelCase.
     */
    QSettings *settings() const { return mSettings; }

public slots:
    void setShowGrid(bool showGrid);
    void setShowTileObjectOutlines(bool enabled);
    void setShowTileAnimations(bool enabled);
    void setSnapToGrid(bool snapToGrid);
    void setSnapToFineGrid(bool snapToFineGrid);
    void setGridColor(QColor gridColor);
    void setGridFine(int gridFine);
    void setObjectLineWidth(qreal lineWidth);
    void setHighlightCurrentLayer(bool highlight);
    void setShowTilesetGrid(bool showTilesetGrid);

signals:
    void showGridChanged(bool showGrid);
    void showTileObjectOutlinesChanged(bool enabled);
    void showTileAnimationsChanged(bool enabled);
    void snapToGridChanged(bool snapToGrid);
    void snapToFineGridChanged(bool snapToFineGrid);
    void gridColorChanged(QColor gridColor);
    void gridFineChanged(int gridFine);
    void objectLineWidthChanged(qreal lineWidth);
    void highlightCurrentLayerChanged(bool highlight);
    void showTilesetGridChanged(bool showTilesetGrid);

    void useOpenGLChanged(bool useOpenGL);

    void objectTypesChanged();

    void mapsDirectoryChanged();

private:
    Preferences();
    ~Preferences();

    bool boolValue(const char *key, bool def = false) const;
    QColor colorValue(const char *key, const QColor &def = QColor()) const;
    QString stringValue(const char *key, const QString &def = QString()) const;
    int intValue(const char *key, int defaultValue) const;
    qreal realValue(const char *key, qreal defaultValue) const;

    QSettings *mSettings;

    bool mShowGrid;
    bool mShowTileObjectOutlines;
    bool mShowTileAnimations;
    bool mSnapToGrid;
    bool mSnapToFineGrid;
    QColor mGridColor;
    int mGridFine;
    qreal mObjectLineWidth;
    bool mHighlightCurrentLayer;
    bool mShowTilesetGrid;

    Map::LayerDataFormat mLayerDataFormat;
    bool mDtdEnabled;
    QString mLanguage;
    bool mReloadTilesetsOnChange;
    bool mUseOpenGL;
    ObjectTypes mObjectTypes;

    bool mAutoMapDrawing;

    QString mMapsDirectory;

    static Preferences *mInstance;
};

} // namespace Internal
} // namespace Tiled

#endif // PREFERENCES_H
