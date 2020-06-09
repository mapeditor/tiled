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

#pragma once

#include "tileset.h"
#include "tilesetchanges.h"

#include <QDialog>

namespace Ui {
class NewTilesetDialog;
}

namespace Tiled {

/**
 * A dialog for the creation of a new tileset, or for editing the parameters
 * of an existing one.
 */
class NewTilesetDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        CreateTileset,
        EditTilesetParameters,
    };

    NewTilesetDialog(QWidget *parent = nullptr);
    ~NewTilesetDialog();

    void setImagePath(const QString &path);
    void setTileSize(QSize size);

    SharedTileset createTileset();

    bool isEmbedded() const;

    bool editTilesetParameters(TilesetParameters &parameters);

private:
    void browse();
    void nameEdited(const QString &name);
    void tilesetTypeChanged(int index);
    void updateOkButton();
    void updateColorPickerButton();
    void tryAccept();
    void pickColorFromImage();
    void colorSelected(QColor);

    void setMode(Mode mode);

    Mode mMode;
    QString mPath;
    Ui::NewTilesetDialog *mUi;
    bool mNameWasEdited;
    SharedTileset mNewTileset;
};

} // namespace Tiled
