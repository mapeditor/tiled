/*
 * saveasimagedialog.cpp
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

#include "saveasimagedialog.h"
#include "ui_saveasimagedialog.h"

#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "imagelayer.h"
#include "objectgroup.h"
#include "preferences.h"
#include "tilelayer.h"
#include "utils.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QImageWriter>
#include <QSettings>

static const char * const VISIBLE_ONLY_KEY = "SaveAsImage/VisibleLayersOnly";
static const char * const CURRENT_SCALE_KEY = "SaveAsImage/CurrentScale";
static const char * const DRAW_GRID_KEY = "SaveAsImage/DrawGrid";

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
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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

    // Restore previously used settings
    QSettings *s = Preferences::instance()->settings();
    const bool visibleLayersOnly =
            s->value(QLatin1String(VISIBLE_ONLY_KEY), true).toBool();
    const bool useCurrentScale =
            s->value(QLatin1String(CURRENT_SCALE_KEY), true).toBool();
    const bool drawTileGrid =
            s->value(QLatin1String(DRAW_GRID_KEY), false).toBool();

    mUi->visibleLayersOnly->setChecked(visibleLayersOnly);
    mUi->currentZoomLevel->setChecked(useCurrentScale);
    mUi->drawTileGrid->setChecked(drawTileGrid);

    connect(mUi->browseButton, SIGNAL(clicked()), SLOT(browse()));
    connect(mUi->fileNameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(updateAcceptEnabled()));

    Utils::restoreGeometry(this);
}

SaveAsImageDialog::~SaveAsImageDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

static bool objectLessThan(const MapObject *a, const MapObject *b)
{
    return a->y() < b->y();
}

void SaveAsImageDialog::accept()
{
    const QString fileName = mUi->fileNameEdit->text();
    if (fileName.isEmpty())
        return;

    if (QFile::exists(fileName)) {
        const QMessageBox::StandardButton button =
                QMessageBox::warning(this,
                                     tr("Save as Image"),
                                     tr("%1 already exists.\n"
                                        "Do you want to replace it?")
                                     .arg(QFileInfo(fileName).fileName()),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);

        if (button != QMessageBox::Yes)
            return;
    }

    const bool visibleLayersOnly = mUi->visibleLayersOnly->isChecked();
    const bool useCurrentScale = mUi->currentZoomLevel->isChecked();
    const bool drawTileGrid = mUi->drawTileGrid->isChecked();

    MapRenderer *renderer = mMapDocument->renderer();

    // Remember the current render flags
    const Tiled::RenderFlags renderFlags = renderer->flags();

    renderer->setFlag(ShowTileObjectOutlines, false);

    QSize mapSize = renderer->mapSize();
    if (useCurrentScale)
        mapSize *= mCurrentScale;

    QImage image(mapSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);

    if (useCurrentScale && mCurrentScale != qreal(1)) {
        painter.setRenderHints(QPainter::SmoothPixmapTransform |
                               QPainter::HighQualityAntialiasing);
        painter.setTransform(QTransform::fromScale(mCurrentScale,
                                                   mCurrentScale));
        renderer->setPainterScale(mCurrentScale);
    } else
        renderer->setPainterScale(1);

    foreach (const Layer *layer, mMapDocument->map()->layers()) {
        if (visibleLayersOnly && !layer->isVisible())
            continue;

        painter.setOpacity(layer->opacity());

        const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(layer);
        const ObjectGroup *objGroup = dynamic_cast<const ObjectGroup*>(layer);
        const ImageLayer *imageLayer = dynamic_cast<const ImageLayer*>(layer);

        if (tileLayer) {
            renderer->drawTileLayer(&painter, tileLayer);
        } else if (objGroup) {
            QList<MapObject*> objects = objGroup->objects();

            if (objGroup->drawOrder() == ObjectGroup::TopDownOrder)
                qStableSort(objects.begin(), objects.end(), objectLessThan);

            foreach (const MapObject *object, objects) {
                if (object->isVisible()) {
                    const QColor color = MapObjectItem::objectColor(object);
                    renderer->drawMapObject(&painter, object, color);
                }
            }
        } else if (imageLayer) {
            renderer->drawImageLayer(&painter, imageLayer);
        }
    }

    if (drawTileGrid) {
        Preferences *prefs = Preferences::instance();
        renderer->drawGrid(&painter, QRectF(QPointF(), renderer->mapSize()),
                           prefs->gridColor());
    }

    // Restore the previous render flags
    renderer->setFlags(renderFlags);

    image.save(fileName);
    mPath = QFileInfo(fileName).path();

    // Store settings for next time
    QSettings *s = Preferences::instance()->settings();
    s->setValue(QLatin1String(VISIBLE_ONLY_KEY), visibleLayersOnly);
    s->setValue(QLatin1String(CURRENT_SCALE_KEY), useCurrentScale);
    s->setValue(QLatin1String(DRAW_GRID_KEY), drawTileGrid);

    QDialog::accept();
}

void SaveAsImageDialog::browse()
{
    // Don't confirm overwrite here, since we'll confirm when the user presses
    // the Save button
    const QString filter = Utils::writableImageFormatsFilter();
    QString f = QFileDialog::getSaveFileName(this, tr("Image"),
                                             mUi->fileNameEdit->text(),
                                             filter, 0,
                                             QFileDialog::DontConfirmOverwrite);
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
