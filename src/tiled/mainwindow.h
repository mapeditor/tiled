/*
 * mainwindow.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
class QuickStampManager;
class StampBrush;
class TerrainBrush;
class TerrainDock;
class TileAnimationEditor;
class TileCollisionEditor;
class TilesetDock;
class ToolManager;
class Zoomable;

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
     * Opens the given file. When opened succesfully, the file is added to the
     * list of recent files.
     *
     * When a \a reader is given, it is used to open the file. Otherwise, a
     * reader is searched using MapReaderInterface::supportsFile.
     *
     * @return whether the file was succesfully opened
     */
    bool openFile(const QString &fileName, MapReaderInterface *reader);

    /**
     * Attempt to open the previously opened file.
     * TODO: Opening last file should be optional
     */
    void openLastFiles();

public slots:
    bool openFile(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);

    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

public slots:
    void newMap();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void saveAsImage();
    void exportAs();
    void closeFile();
    void closeAllFiles();

    void cut();
    void copy();
    void paste();
    void delete_(); // delete is a reserved word
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
    void aboutTiled();
    void openRecentFile();
    void clearRecentFiles();

    void flipHorizontally() { flip(FlipHorizontally); }
    void flipVertically() { flip(FlipVertically); }
    void rotateLeft() { rotate(RotateLeft); }
    void rotateRight() { rotate(RotateRight); }

    void flip(FlipDirection direction);
    void rotate(RotateDirection direction);

    void setStampBrush(const TileLayer *tiles);
    void setTerrainBrush(const Terrain *terrain);
    void saveQuickStamp(int index);
    void updateStatusInfoLabel(const QString &statusInfo);

    void mapDocumentChanged(MapDocument *mapDocument);
    void closeMapDocument(int index);

    void autoMappingError();
    void autoMappingWarning();

    void onAnimationEditorClosed();
    void onCollisionEditorClosed();

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
     * Save the current map to the given file name. When saved succesfully, the
     * file is added to the list of recent files.
     * @return <code>true</code> on success, <code>false</code> on failure
     */
    bool saveFile(const QString &fileName);

    void writeSettings();
    void readSettings();

    void addMapDocument(MapDocument *mapDocument);
    QStringList recentFiles() const;
    QString fileDialogStartLocation() const;

    void setRecentFile(const QString &fileName);
    void updateRecentFiles();

    void retranslateUi();

    Ui::MainWindow *mUi;
    MapDocument *mMapDocument;
    MapDocumentActionHandler *mActionHandler;
    LayerDock *mLayerDock;
    MapsDock *mMapsDock;
    ObjectsDock *mObjectsDock;
    TilesetDock *mTilesetDock;
    TerrainDock *mTerrainDock;
    MiniMapDock* mMiniMapDock;
    ConsoleDock *mConsoleDock;
    TileAnimationEditor *mTileAnimationEditor;
    TileCollisionEditor *mTileCollisionEditor;
    QLabel *mCurrentLayerLabel;
    Zoomable *mZoomable;
    QComboBox *mZoomComboBox;
    QLabel *mStatusInfoLabel;
    QSettings mSettings;
    QToolButton *mRandomButton;
    CommandButton *mCommandButton;

    StampBrush *mStampBrush;
    BucketFillTool *mBucketFillTool;
    TerrainBrush *mTerrainBrush;

    enum { MaxRecentFiles = 8 };
    QAction *mRecentFiles[MaxRecentFiles];

    QMenu *mLayerMenu;
    QAction *mViewsAndToolbarsMenu;
    QAction *mShowTileAnimationEditor;
    QAction *mShowTileCollisionEditor;

    void setupQuickStamps();

    AutomappingManager *mAutomappingManager;
    DocumentManager *mDocumentManager;
    QuickStampManager *mQuickStampManager;
    ToolManager *mToolManager;
};

} // namespace Internal
} // namespace Tiled

#endif // MAINWINDOW_H
