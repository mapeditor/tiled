/*
 * newtilesetdialog.h
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef NEWTILESETDIALOG_H
#define NEWTILESETDIALOG_H

#include <QDialog>

namespace Ui {
class NewTilesetDialog;
}

namespace Tiled {

class Tileset;

namespace Internal {

/**
 * A dialog for the creation of a new tileset.
 */
class NewTilesetDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructs a new tileset dialog
     *
     * @param path the path to start in by default, or an image file
     */
    NewTilesetDialog(const QString &path, QWidget *parent = 0);
    ~NewTilesetDialog();

    void setTileWidth(int width);
    void setTileHeight(int height);

    /**
     * Shows the dialog and returns the created tileset. Returns 0 if the
     * dialog was cancelled.
     */
    Tileset *createTileset();

private slots:
    void browse();
    void nameEdited(const QString &name);
    void tilesetTypeChanged(int index);
    void updateOkButton();
    void tryAccept();

private:
    QString mPath;
    Ui::NewTilesetDialog *mUi;
    bool mNameWasEdited;
    Tileset *mNewTileset;
};

} // namespace Internal
} // namespace Tiled

#endif // NEWTILESETDIALOG_H
