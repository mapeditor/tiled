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

namespace Ui {
class MainWindow;
}

namespace Tiled {

class FileFormat;
class TileLayer;
class Terrain;

namespace Internal {

class AutomappingManager;
class CommandButton;
class DocumentManager;
class MapDocumentActionHandler;
class MapScene;
class MapView;
class ObjectTypesEditor;
class TmxMapFormat;
class TsxTilesetFormat;

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
    MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
    ~MainWindow();

    void commitData(QSessionManager &manager);

    /**
     * Opens the given file. When opened successfully, the file is added to the
     * list of recent files.
     *
     * When a \a format is given, it is used to open the file. Otherwise, a
     * format is searched using MapFormat::supportsFile.
     *
     * @return whether the file was successfully opened
     */
    bool openFile(const QString &fileName, FileFormat *fileFormat);

    /**
     * Attempt to open the previously opened file.
     */
    void openLastFiles();

public slots:
    bool openFile(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;

    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;

private slots:
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
    void copy();
    void paste();
    void delete_(); // 'delete' is a reserved word
    void openPreferences();

    void labelVisibilityActionTriggered(QAction *action);
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
    void openDocumentation();
    void becomePatron();
    void aboutTiled();
    void openRecentFile();
    void clearRecentFiles();

    void mapDocumentChanged(Document *document);
    void closeMapDocument(int index);

    void reloadError(const QString &error);
    void autoMappingError(bool automatic);
    void autoMappingWarning(bool automatic);

    void onObjectTypesEditorClosed();
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
    bool confirmSave(Document *document);

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
    ConsoleDock *mConsoleDock;
    ObjectTypesEditor *mObjectTypesEditor;
    QSettings mSettings;
    QToolButton *mRandomButton;
    CommandButton *mCommandButton;

    enum { MaxRecentFiles = 8 };
    QAction *mRecentFiles[MaxRecentFiles];

    QMenu *mLayerMenu;
    QAction *mViewsAndToolbarsMenu;
    QAction *mShowObjectTypesEditor;
    QAction *mShowTileAnimationEditor;
    QAction *mShowTileCollisionEditor;

    void setupQuickStamps();

    AutomappingManager *mAutomappingManager;
    DocumentManager *mDocumentManager;

    TmxMapFormat *mTmxMapFormat;
    TsxTilesetFormat *mTsxTilesetFormat;
};

} // namespace Internal
} // namespace Tiled

#endif // MAINWINDOW_H
