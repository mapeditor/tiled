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

#include <QMainWindow>
#include <QSettings>
#include "ui_mainwindow.h"

class QUndoStack;

namespace Tiled {
namespace Internal {

class MapScene;
class LayerDock;

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
     */
    void openFile(const QString &fileName);

    /**
     * Save the current map to the given file name. When saved succesfully, the
     * file is added to the list of recent files.
     * @return <code>true</code> on success, <code>false</code> on failure
     */
    bool saveFile(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void resizeMap();
    void editMapProperties();
    void updateModified();
    void aboutTiled();
    void openRecentFile();
    void clearRecentFiles();

private:
    /**
      * Asks the user whether the map should be saved when necessary.
      *
      * @return <code>true</code> when any unsaved data is either discarded or
      *         saved, <code>false</code> when the user cancelled or saving
      *         failed.
      */
    bool confirmSave();
    void updateActions();
    void writeSettings();
    void readSettings();
    void setCurrentFileName(const QString &fileName);
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

    Ui::MainWindowClass mUi;
    MapScene *mScene;
    LayerDock *mLayerDock;
    QSettings mSettings;
    QString mCurrentFileName;
    QUndoStack *mUndoStack;

    enum { MaxRecentFiles = 8 };
    QAction *mRecentFiles[MaxRecentFiles];
};

} // namespace Internal
} // namespace Tiled

#endif // MAINWINDOW_H
