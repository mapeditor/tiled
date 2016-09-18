/*
 * mainwindow.h
 * Copyright 2008-2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mapdocument.h"
#include "consoledock.h"

#include <QMainWindow>
#include <QSessionManager>
#include <QSettings>

class QComboBox;
class QLabel;
class QToolButton;
class QShortcut;

namespace Ui {
class MainWindow;
}

namespace Tiled {

class TileLayer;
class Terrain;
class MapReaderInterface;

namespace Internal {

class AutomappingManager;
class BucketFillTool;
class CommandButton;
class DocumentManager;
class LayerDock;
class MapDocumentActionHandler;
class MapScene;
class MapsDock;
class MapView;
class MiniMapDock;
class ObjectsDock;
class PropertiesDock;
class StampBrush;
class TerrainBrush;
class TerrainDock;
class TileAnimationEditor;
class TileCollisionEditor;
class TilesetDock;
class TileStamp;
class TileStampManager;
class ToolManager;
class Zoomable;
class ObjectSelectionTool;
class RTBValidatorDock;
class RTBValidator;
class RTBTileSelectionManager;
class RTBSelectAreaTool;
class RTBInsertTool;
class RTBTutorial;
class RTBTutorialDock;

/**
 * The main editor window.
 *
 * Represents the main user interface, including the menu bar. It keeps track
 * of the current file and is also the entry point of all menu actions.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~MainWindow();

    void commitData(QSessionManager &manager);

    /**
     * Opens the given file. When opened successfully, the file is added to the
     * list of recent files.
     *
     * When a \a reader is given, it is used to open the file. Otherwise, a
     * reader is searched using MapReaderInterface::supportsFile.
     *
     * @return whether the file was successfully opened
     */
    bool openFile(const QString &fileName, MapReaderInterface *reader);

    /**
     * Attempt to open the previously opened file.
     */
    void openLastFiles();

    bool showPropVisualization() { return mShowPropVisualization; }

public slots:
    bool openFile(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);

    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

    void paintEvent(QPaintEvent * event);

public slots:
    void newMap();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void saveAll();
    void export_(); // 'export' is a reserved word
    void exportAs();
    void exportAsImage();
    void reload();
    void closeFile();
    void closeAllFiles();

    void cut();
    void copy(bool isCut = false);
    void paste();
    void delete_(); // 'delete' is a reserved word
    void openPreferences();

    void zoomIn();
    void zoomOut();
    void zoomNormal();

    bool newTileset(const QString &path = QString());
    void newTilesets(const QStringList &paths);
    void reloadTilesets();
    void addExternalTileset();
    void resizeMap();
    void offsetMap();
    void editMapProperties();

    void updateWindowTitle();
    void updateActions();
    void updateZoomLabel();
    void becomePatron();
    void aboutTiled();
    void openRecentFile();
    void clearRecentFiles();

    void flipHorizontally() { flip(FlipHorizontally); }
    void flipVertically() { flip(FlipVertically); }
    void rotateLeft() { rotate(RotateLeft); }
    void rotateRight() { rotate(RotateRight); }

    void flip(FlipDirection direction);
    void rotate(RotateDirection direction);

    void setStamp(const TileStamp &stamp);
    void setTerrainBrush(const Terrain *terrain);
    void updateStatusInfoLabel(const QString &statusInfo);

    void mapDocumentChanged(MapDocument *mapDocument);
    void closeMapDocument(int index);

    void reloadError(const QString &error);

    void activateObjectSelectionTool();
    void activateObjectSelectionTool(MapObject *mapObject);
    bool saveFileAsJSON();

private slots:
    void setShowPropVisualization(bool show);
    void buildMap();
    void highlightSection(int section);

private:
    /**
      * Asks the user whether the given \a mapDocument should be saved, when
      * necessary. If it needs to ask, also makes sure that it is the current
      * document.
      *
      * @return <code>true</code> when any unsaved data is either discarded or
      *         saved, <code>false</code> when the user cancelled or saving
      *         failed.
      */
    bool confirmSave(MapDocument *mapDocument);

    /**
      * Checks all maps for changes, if so, ask if to save these changes.
      *
      * @return <code>true</code> when any unsaved data is either discarded or
      *         saved, <code>false</code> when the user cancelled or saving
      *         failed.
      */
    bool confirmAllSave();

    /**
     * Save the current map to the given file name. When saved successfully, the
     * file is added to the list of recent files.
     * @return <code>true</code> on success, <code>false</code> on failure
     */
    bool saveFile(const QString &fileName);

    void writeSettings();
    void readSettings();

    QStringList recentFiles() const;
    QString fileDialogStartLocation() const;

    void setRecentFile(const QString &fileName);
    void updateRecentFiles();

    void retranslateUi();

    Ui::MainWindow *mUi;
    MapDocument *mMapDocument;
    MapDocumentActionHandler *mActionHandler;
    PropertiesDock *mPropertiesDock;
    LayerDock *mLayerDock;
    MapsDock *mMapsDock;
    MiniMapDock* mMiniMapDock;
    QLabel *mCurrentLayerLabel;
    Zoomable *mZoomable;
    QComboBox *mZoomComboBox;
    QLabel *mStatusInfoLabel;
    QSettings *mSettings;

    StampBrush *mStampBrush;
    BucketFillTool *mBucketFillTool;
    TerrainBrush *mTerrainBrush;
    ObjectSelectionTool *mObjectSelectionTool;
    RTBSelectAreaTool *mSelectAreaTool;
    RTBInsertTool *mInsertTool;

    enum { MaxRecentFiles = 8 };
    QAction *mRecentFiles[MaxRecentFiles];

    QMenu *mLayerMenu;
    QAction *mViewsAndToolbarsMenu;
    QAction *mShowTileAnimationEditor;
    QAction *mShowTileCollisionEditor;

    void setupQuickStamps();

    DocumentManager *mDocumentManager;
    ToolManager *mToolManager;
    TileStampManager *mTileStampManager;

    RTBTileSelectionManager *mTileSelectionManager;
    QShortcut *mFloorLayerShortcut;
    QShortcut *mOrbLayerShortcut;
    QShortcut *mObjectLayerShortcut;
    QShortcut *mIntervalSpeedShortcut1;
    QShortcut *mIntervalSpeedShortcut2;
    QShortcut *mIntervalSpeedShortcut3;
    QShortcut *mIntervalSpeedShortcut4;
    QShortcut *mIntervalOffsetShortcut1;
    QShortcut *mIntervalOffsetShortcut2;
    QShortcut *mIntervalOffsetShortcut3;
    QShortcut *mIntervalOffsetShortcut4;
    QShortcut *mIntervalOffsetShortcut5;
    QShortcut *mIntervalOffsetShortcut6;
    QShortcut *mIntervalOffsetShortcut7;
    QShortcut *mIntervalOffsetShortcut8;
    QShortcut *mChangeLayerShortcut;
    QShortcut *mChangeLayerBackShortcut;
    RTBValidatorDock *mValidatorDock;
    RTBValidator *mValidator;
    QAction *mPlayLevelAction;
    QAction *mShowMapProperties;
    RTBTutorial *mTutorial;
    RTBTutorialDock *mTutorialDock;

    bool mShowPropVisualization;
    int mHighlightSection;
};

} // namespace Internal
} // namespace Tiled

#endif // MAINWINDOW_H
