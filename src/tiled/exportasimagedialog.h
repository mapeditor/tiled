/*
 * exportasimagedialog.h
 * Copyright 2009-2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tilededitor_global.h"

#include <QDialog>

namespace Ui {
class ExportAsImageDialog;
}

namespace Tiled {

class MapDocument;

/**
 * The dialog for exporting a map as an image.
 */
class TILED_EDITOR_EXPORT ExportAsImageDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Creates an Export As Image dialog. The suggested name for the image will
     * be based on the given \a fileName. Use \a currentScale to specify the
     * current zoom level of the map view.
     */
    ExportAsImageDialog(MapDocument *mapDocument,
                        const QString &fileName,
                        qreal currentScale,
                        QWidget *parent = nullptr);
    ~ExportAsImageDialog() override;

    /**
     * Returns the rectangle of the selected area in the final exported image.
     * This is based on the same scaling and centering logic as the rendered map.
     */
    static QRect selectedAreaImageRect(const QRect &mapBounds,
                                       const QRect &selectionBounds,
                                       const QSize &imageSize);

public:
    void accept() override;

private:
    void browse();
    void updateAcceptEnabled();

    Ui::ExportAsImageDialog *mUi;
    MapDocument *mMapDocument;
    qreal mCurrentScale;

    static QString mPath;
};

} // namespace Tiled
