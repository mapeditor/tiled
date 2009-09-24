/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
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

#include "saveasimagedialog.h"
#include "ui_saveasimagedialog.h"

#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "tilelayer.h"
#include "utils.h"

#include <QFileDialog>
#include <QImageWriter>

using namespace Tiled;
using namespace Tiled::Internal;

QString SaveAsImageDialog::mPath;

SaveAsImageDialog::SaveAsImageDialog(MapDocument *mapDocument,
                                     const QString &fileName,
                                     qreal currentScale,
                                     QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::SaveAsImageDialog)
    , mMapDocument(mapDocument)
    , mCurrentScale(currentScale)
{
    mUi->setupUi(this);

    // Default to the last chosen location
    QString suggestion = mPath;

    // Suggest a nice name for the image
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        const QString path = fileInfo.path();
        const QString baseName = fileInfo.completeBaseName();

        if (suggestion.isEmpty())
            suggestion = path;

        suggestion += QLatin1Char('/');
        suggestion += baseName;
        suggestion += QLatin1String(".png");
    } else {
        suggestion += QLatin1Char('/');
        suggestion += QLatin1String("map.png");
    }

    mUi->fileNameEdit->setText(suggestion);

    connect(mUi->browseButton, SIGNAL(clicked()), SLOT(browse()));
    connect(mUi->fileNameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(updateAcceptEnabled()));
}

SaveAsImageDialog::~SaveAsImageDialog()
{
    delete mUi;
}

void SaveAsImageDialog::accept()
{
    const QString fileName = mUi->fileNameEdit->text();
    if (fileName.isEmpty())
        return;

    const bool visibleLayersOnly = mUi->visibleLayersOnly->isChecked();
    const bool useCurrentScale = mUi->currentZoomLevel->isChecked();

    MapRenderer *renderer = mMapDocument->renderer();
    QSize mapSize = renderer->mapSize();

    if (useCurrentScale)
        mapSize *= mCurrentScale;

    QImage image(mapSize, QImage::Format_ARGB32);
    QPainter painter(&image);

    if (useCurrentScale && mCurrentScale != qreal(1)) {
        painter.setRenderHints(QPainter::SmoothPixmapTransform |
                               QPainter::HighQualityAntialiasing);
        painter.setTransform(QTransform::fromScale(mCurrentScale,
                                                   mCurrentScale));
    }

    foreach (const Layer *layer, mMapDocument->map()->layers()) {
        if (const TileLayer *tl = dynamic_cast<const TileLayer*>(layer)) {
            if (!visibleLayersOnly || tl->isVisible())
                renderer->drawTileLayer(&painter, tl);
        }
    }

    image.save(fileName);
    mPath = QFileInfo(fileName).path();

    QDialog::accept();
}

void SaveAsImageDialog::browse()
{
    const QString filter = Utils::writableImageFormatsFilter();
    QString f = QFileDialog::getSaveFileName(this, tr("Image"),
                                             mUi->fileNameEdit->text(),
                                             filter);
    if (!f.isEmpty()) {
        mUi->fileNameEdit->setText(f);
        mPath = f;
    }
}

void SaveAsImageDialog::updateAcceptEnabled()
{
    QPushButton *saveButton = mUi->buttonBox->button(QDialogButtonBox::Save);
    saveButton->setEnabled(!mUi->fileNameEdit->text().isEmpty());
}
