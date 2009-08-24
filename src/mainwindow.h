/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mapdocument.h"

#include <QMainWindow>
#include <QSettings>
#include "ui_mainwindow.h"

class QUndoGroup;

namespace Tiled {
namespace Internal {

class MapScene;
class LayerDock;
class TilesetDock;

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
    MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~MainWindow();

    void commitData(QSessionManager &manager);

    /**
     * Opens the given file. When opened succesfully, the file is added to the
     * list of recent files.
     *
     * @return whether the file was succesfully opened
     */
    bool openFile(const QString &fileName);

    /**
     * Attempt to open the previously opened file.
     * TODO: Opening last file should be optional
     */
    void openLastFile();

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);

private slots:
    void newMap();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void closeFile();
    void newTileset();
    void resizeMap();
    void editMapProperties();
    void updateModified();
    void updateActions();
    void aboutTiled();
    void openRecentFile();
    void clearRecentFiles();

    void selectAll();
    void selectNone();
    void addTileLayer();
    void addObjectLayer();
    void duplicateLayer();
    void moveLayerUp();
    void moveLayerDown();
    void removeLayer();
    void editLayerProperties();

private:
    /**
      * Asks the user whether the map should be saved when necessary.
      *
      * @return <code>true</code> when any unsaved data is either discarded or
      *         saved, <code>false</code> when the user cancelled or saving
      *         failed.
      */
    bool confirmSave();

    /**
     * Save the current map to the given file name. When saved succesfully, the
     * file is added to the list of recent files.
     * @return <code>true</code> on success, <code>false</code> on failure
     */
    bool saveFile(const QString &fileName);

    void writeSettings();
    void readSettings();
    void setCurrentFileName(const QString &fileName);
    void setMapDocument(MapDocument *mapDocument);
    QStringList recentFiles() const;
    QString fileDialogStartLocation() const;

    /**
     * Add the given file to the recent files list.
     */
    void setRecentFile(const QString &fileName);

    /**
     * Update the recent files menu.
     */
    void updateRecentFiles();

    void addLayer(MapDocument::LayerType type);

    Ui::MainWindowClass mUi;
    MapDocument *mMapDocument;
    MapScene *mScene;
    LayerDock *mLayerDock;
    TilesetDock *mTilesetDock;
    QSettings mSettings;
    QString mCurrentFileName;
    QUndoGroup *mUndoGroup;

    enum { MaxRecentFiles = 8 };
    QAction *mRecentFiles[MaxRecentFiles];
};

} // namespace Internal
} // namespace Tiled

#endif // MAINWINDOW_H
