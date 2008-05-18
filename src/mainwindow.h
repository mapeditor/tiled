/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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

#include <QtGui/QMainWindow>
#include <QSettings>
#include "ui_mainwindow.h"

namespace Tiled {
namespace Internal {

class MapScene;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~MainWindow();

private slots:
    void openFile();
    void saveFile();
    void resizeMap();
    void aboutTiled();
    void openRecentFile();

private:
    void openFile(const QString& fileName);
    void writeSettings();
    void readSettings();

    /**
     * Update the recent files menu.
     */
    void updateRecentFiles();

    /**
     * Add the given file to the recent files list.
     */
    void setRecentFile(const QString &fileName);

    Ui::MainWindowClass mUi;
    MapScene *mScene;
    QSettings mSettings;

    enum { MaxRecentFiles = 4 };
    QAction *mRecentFiles[MaxRecentFiles];
};

} // namespace Internal
} // namespace Tiled

#endif // MAINWINDOW_H
