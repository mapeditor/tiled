/*
 * newmapdialog.h
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "mapdocument.h"

#include <QDialog>

namespace Ui {
class NewMapDialog;
}

namespace Tiled {

/**
 * A dialog for the creation of a new map.
 */
class NewMapDialog : public QDialog
{
    Q_OBJECT

public:
    NewMapDialog(QWidget *parent = nullptr);
    ~NewMapDialog();

    /**
     * Shows the dialog and returns the created map. Returns null if the dialog
     * was cancelled.
     */
    MapDocumentPtr createMap();

private:
    void refreshPixelSize();

    void updateWidgets(bool checked);

    Ui::NewMapDialog *mUi;
};

} // namespace Tiled
