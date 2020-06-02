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

#pragma once

#include "clipboardmanager.h"
#include "document.h"
#include "preferences.h"
#include "preferencesdialog.h"
#include "project.h"
#include "session.h"

#include <QMainWindow>
#include <QPointer>
#include <QSessionManager>

class QComboBox;
class QLabel;

namespace Ui {
class MainWindow;
}

namespace Tiled {

class FileFormat;
class TileLayer;
class Terrain;

class AutomappingManager;
class ConsoleDock;
class DocumentManager;
class Editor;
class IssuesDock;
class LocatorWidget;
class MapDocument;
class MapDocumentActionHandler;
class MapEditor;
class MapScene;
class MapView;
class ObjectTypesEditor;
class ProjectDock;
class ProjectModel;
class TilesetDocument;
class TilesetEditor;
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
    MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~MainWindow() override;

    void commitData(QSessionManager &manager);

    void initializeSession();

    /**
     * Opens the given file. When opened successfully, the file is added to the
     * list of recent files.
     *
     * When a \a format is given, it is used to open the file. Otherwise, a
     * format is searched using MapFormat::supportsFile.
     *
     * @return whether the file was successfully opened
     */
    bool openFile(const QString &fileName, FileFormat *fileFormat = nullptr);

    Project &project() const;
    ProjectModel *projectModel() const;

    bool addRecentProjectsActions(QMenu *menu) const;

    static MainWindow *instance();

signals:
    void projectChanged();

protected:
    bool event(QEvent *event) override;

    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;

    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;

private:
    void newMap();
    void openFileDialog();
    void openFileInProject();
    bool saveFile();
    bool saveFileAs();
    void saveAll();
    void export_(); // 'export' is a reserved word
    bool exportDocument(Document *document);
    void exportAs();
    void exportAsImage();
    void reload();
    void closeFile();
    bool closeAllFiles();

    void openProject();
    void openProjectFile(const QString &fileName);
    void saveProjectAs();
    void closeProject();
    void switchProject(Project project);
    void restoreSession();
    void projectProperties();

    void cut();
    void copy();
    void paste();
    void pasteInPlace();
    void delete_();
    void openPreferences();

    void labelVisibilityActionTriggered(QAction *action);
    void zoomIn();
    void zoomOut();
    void zoomNormal();
    void fitInView();
    void setFullScreen(bool fullScreen);
    void toggleClearView(bool clearView);
    void resetToDefaultLayout();

    bool newTileset(const QString &path = QString());
    void reloadTilesetImages();
    void addExternalTileset();
    void resizeMap();
    void offsetMap();
    void editMapProperties();

    void editTilesetProperties();

    void updateWindowTitle();
    void updateActions();
    void updateZoomable();
    void updateZoomActions();
    void openDocumentation();
    void openForum();
    void showDonationDialog();
    void aboutTiled();
    void openRecentFile();
    void reopenClosedFile();
    void openRecentProject();

    void documentChanged(Document *document);
    void documentSaved(Document *document);
    void closeDocument(int index);

    void currentEditorChanged(Editor *editor);

    void reloadError(const QString &error);
    void autoMappingError(bool automatic);
    void autoMappingWarning(bool automatic);

    void onObjectTypesEditorClosed();

    void ensureHasBorderInFullScreen();

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

    bool confirmSaveWorld(const QString &fileName);

    void writeSettings();
    void readSettings();

    void updateRecentFilesMenu();
    void updateRecentProjectsMenu();
    void updateViewsAndToolbarsMenu();

    void retranslateUi();

    void exportMapAs(MapDocument *mapDocument);
    void exportTilesetAs(TilesetDocument *tilesetDocument);

    Ui::MainWindow *mUi;
    Document *mDocument = nullptr;
    Zoomable *mZoomable = nullptr;
    MapDocumentActionHandler *mActionHandler;
    ConsoleDock *mConsoleDock;
    ProjectDock *mProjectDock;
    IssuesDock *mIssuesDock;
    ObjectTypesEditor *mObjectTypesEditor;
    QPointer<LocatorWidget> mLocatorWidget;

    QAction *mRecentFiles[Preferences::MaxRecentFiles];

    QMenu *mLayerMenu;
    QMenu *mNewLayerMenu;
    QMenu *mGroupLayerMenu;
    QMenu *mViewsAndToolbarsMenu;
    QAction *mViewsAndToolbarsAction;
    QAction *mShowObjectTypesEditor;

    QAction *mResetToDefaultLayout;

    void setupQuickStamps();

    AutomappingManager *mAutomappingManager;
    DocumentManager *mDocumentManager;
    MapEditor *mMapEditor;
    TilesetEditor *mTilesetEditor;
    QList<QWidget*> mEditorStatusBarWidgets;

    QPointer<PreferencesDialog> mPreferencesDialog;

    QMap<QMainWindow*, QByteArray> mMainWindowStates;

    SessionOption<QStringList> mLoadedWorlds { "loadedWorlds" };

    static MainWindow *mInstance;
};

inline MainWindow *MainWindow::instance()
{
    Q_ASSERT(mInstance);
    return mInstance;
}

} // namespace Tiled
