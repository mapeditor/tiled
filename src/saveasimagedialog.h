/*
 * saveasimagedialog.h
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

#ifndef SAVEASIMAGEDIALOG_H
#define SAVEASIMAGEDIALOG_H

#include <QDialog>

namespace Ui {
class SaveAsImageDialog;
}

namespace Tiled {
namespace Internal {

class MapDocument;

/**
 * The dialog for saving a map as an image.
 */
class SaveAsImageDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Creates a Save As Image dialog. The suggested name for the image will
     * be based on the given \a fileName. Use \a currentScale to specify the
     * current zoom level of the map view.
     */
    SaveAsImageDialog(MapDocument *mapDocument,
                      const QString &fileName,
                      qreal currentScale,
                      QWidget *parent = 0);
    ~SaveAsImageDialog();

public:
    void accept();

private slots:
    void browse();
    void updateAcceptEnabled();

private:
    Ui::SaveAsImageDialog *mUi;
    MapDocument *mMapDocument;
    qreal mCurrentScale;
    static QString mPath;
};

} // namespace Internal
} // namespace Tiled

#endif // SAVEASIMAGEDIALOG_H
