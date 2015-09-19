/*
 * exportasimagedialog.cpp
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

#include "exportasimagedialog.h"
#include "ui_exportasimagedialog.h"

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
static const char * const INCLUDE_BACKGROUND_COLOR = "SaveAsImage/IncludeBackgroundColor";
static const char * const SPLIT_INTO_TILES_KEY = "SaveAsImage/SplitIntoTiles";

using namespace Tiled;
using namespace Tiled::Internal;

QString ExportAsImageDialog::mPath;

ExportAsImageDialog::ExportAsImageDialog(MapDocument *mapDocument,
                                         const QString &fileName,
                                         qreal currentScale,
                                         QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::ExportAsImageDialog)
    , mMapDocument(mapDocument)
    , mCurrentScale(currentScale)
{
    mUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QPushButton *saveButton = mUi->buttonBox->button(QDialogButtonBox::Save);
    saveButton->setText(tr("Export"));

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

    // Constrain tile width/height to map size
    const QSize mapSize = mMapDocument->renderer()->mapSize();
    mUi->tileWidthSpinBox->setMaximum(mapSize.width());
    mUi->tileHeightSpinBox->setMaximum(mapSize.height());

    // Restore previously used settings
    QSettings *s = Preferences::instance()->settings();
    const bool visibleLayersOnly =
            s->value(QLatin1String(VISIBLE_ONLY_KEY), true).toBool();
    const bool useCurrentScale =
            s->value(QLatin1String(CURRENT_SCALE_KEY), true).toBool();
    const bool drawTileGrid =
            s->value(QLatin1String(DRAW_GRID_KEY), false).toBool();
    const bool includeBackgroundColor =
            s->value(QLatin1String(INCLUDE_BACKGROUND_COLOR), false).toBool();
    const bool splitIntoTiles =
            s->value(QLatin1String(SPLIT_INTO_TILES_KEY), false).toBool();

    mUi->visibleLayersOnly->setChecked(visibleLayersOnly);
    mUi->currentZoomLevel->setChecked(useCurrentScale);
    mUi->drawTileGrid->setChecked(drawTileGrid);
    mUi->includeBackgroundColor->setChecked(includeBackgroundColor);
    mUi->splitIntoTiles->setChecked(splitIntoTiles);
    mUi->tileWidthSpinBox->setEnabled(splitIntoTiles);
    mUi->tileHeightSpinBox->setEnabled(splitIntoTiles);

    connect(mUi->browseButton, SIGNAL(clicked()), SLOT(browse()));
    connect(mUi->fileNameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(updateAcceptEnabled()));
    connect(mUi->splitIntoTiles, SIGNAL(clicked(bool)),
            mUi->tileWidthSpinBox, SLOT(setEnabled(bool)));
    connect(mUi->splitIntoTiles, SIGNAL(clicked(bool)),
            mUi->tileHeightSpinBox, SLOT(setEnabled(bool)));

    Utils::restoreGeometry(this);
}

ExportAsImageDialog::~ExportAsImageDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

static bool objectLessThan(const MapObject *a, const MapObject *b)
{
    return a->y() < b->y();
}

static bool smoothTransform(qreal scale)
{
    return scale != qreal(1) && scale < qreal(2);
}

/*!
    Returns a rectangular portion of \a image specified by \a rect,
    without copying. \a image must exist as long as the returned
    image does.

    http://stackoverflow.com/a/12682022/904422
*/
static QImage createSubImage(QImage* image, const QRect &rect) {
    const size_t offset = rect.x() * image->depth() / 8
            + rect.y() * image->bytesPerLine();
    return QImage(image->bits() + offset, rect.width(), rect.height(),
                  image->bytesPerLine(), image->format());
}

void ExportAsImageDialog::accept()
{
    const QString fileName = mUi->fileNameEdit->text();
    if (fileName.isEmpty())
        return;

    const bool visibleLayersOnly = mUi->visibleLayersOnly->isChecked();
    const bool useCurrentScale = mUi->currentZoomLevel->isChecked();
    const bool drawTileGrid = mUi->drawTileGrid->isChecked();
    const bool includeBackgroundColor = mUi->includeBackgroundColor->isChecked();
    const bool splitIntoTiles = mUi->splitIntoTiles->isChecked();

    if (QFile::exists(fileName) && !splitIntoTiles) {
        const QMessageBox::StandardButton button =
                QMessageBox::warning(this,
                                     tr("Export as Image"),
                                     tr("%1 already exists.\n"
                                        "Do you want to replace it?")
                                     .arg(QFileInfo(fileName).fileName()),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);

        if (button != QMessageBox::Yes)
            return;
    }

    MapRenderer *renderer = mMapDocument->renderer();

    // Remember the current render flags
    const Tiled::RenderFlags renderFlags = renderer->flags();

    renderer->setFlag(ShowTileObjectOutlines, false);

    QSize mapSize = renderer->mapSize();

    QMargins margins = mMapDocument->map()->computeLayerOffsetMargins();
    mapSize.setWidth(mapSize.width() + margins.left() + margins.right());
    mapSize.setHeight(mapSize.height() + margins.top() + margins.bottom());

    if (useCurrentScale)
        mapSize *= mCurrentScale;

    QImage image;

    try {
        image = QImage(mapSize, QImage::Format_ARGB32_Premultiplied);

        if (includeBackgroundColor) {
            if (mMapDocument->map()->backgroundColor().isValid())
                image.fill(mMapDocument->map()->backgroundColor());
            else
                image.fill(Qt::gray);
        } else {
            image.fill(Qt::transparent);
        }
    } catch (const std::bad_alloc &) {
        QMessageBox::critical(this,
                              tr("Out of Memory"),
                              tr("Could not allocate sufficient memory for the image. "
                                 "Try reducing the zoom level or using a 64-bit version of Tiled."));
        return;
    }

    if (image.isNull()) {
        const size_t gigabyte = 1073741824;
        const size_t memory = size_t(mapSize.width()) * size_t(mapSize.height()) * 4;
        const double gigabytes = (double) memory / gigabyte;

        QMessageBox::critical(this,
                              tr("Image too Big"),
                              tr("The resulting image would be %1 x %2 pixels and take %3 GB of memory. "
                                 "Tiled is unable to create such an image. Try reducing the zoom level.")
                              .arg(mapSize.width())
                              .arg(mapSize.height())
                              .arg(gigabytes, 0, 'f', 2));
        return;
    }

    QPainter painter(&image);

    if (useCurrentScale) {
        if (smoothTransform(mCurrentScale))
            painter.setRenderHints(QPainter::SmoothPixmapTransform);

        painter.setTransform(QTransform::fromScale(mCurrentScale,
                                                   mCurrentScale));
        renderer->setPainterScale(mCurrentScale);
    } else {
        renderer->setPainterScale(1);
    }

    painter.translate(margins.left(), margins.top());

    foreach (const Layer *layer, mMapDocument->map()->layers()) {
        if (visibleLayersOnly && !layer->isVisible())
            continue;

        painter.setOpacity(layer->opacity());
        painter.translate(layer->offset());

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
                    if (object->rotation() != qreal(0)) {
                        QPointF origin = renderer->pixelToScreenCoords(object->position());
                        painter.save();
                        painter.translate(origin);
                        painter.rotate(object->rotation());
                        painter.translate(-origin);
                    }

                    const QColor color = MapObjectItem::objectColor(object);
                    renderer->drawMapObject(&painter, object, color);

                    if (object->rotation() != qreal(0))
                        painter.restore();
                }
            }
        } else if (imageLayer) {
            renderer->drawImageLayer(&painter, imageLayer);
        }

        painter.translate(-layer->offset());
    }

    if (drawTileGrid) {
        Preferences *prefs = Preferences::instance();
        renderer->drawGrid(&painter, QRectF(QPointF(), renderer->mapSize()),
                           prefs->gridColor());
    }

    // Restore the previous render flags
    renderer->setFlags(renderFlags);

    if (splitIntoTiles) {
        const int extensionIndex = fileName.lastIndexOf(QLatin1Char('.'));
        if (extensionIndex == -1) {
            QMessageBox::information(this,
                                     tr("Export as Image"),
                                     tr("Couldn't find filel extension for %1.\n"
                                        "Unable to export tiled image.")
                                     .arg(QFileInfo(fileName).fileName()),
                                     QMessageBox::Ok);
        } else {
            const int tileWidth = mUi->tileWidthSpinBox->value();
            const int tileHeight = mUi->tileHeightSpinBox->value();
            const int rows = mapSize.height() / tileHeight;
            const int columns = mapSize.width() / tileWidth;

            for (int row = 0; row < rows; ++row) {
                for (int column = 0; column < columns; ++column) {
                    const int index = row * columns + column;
                    const QString tileFileName = QString::fromLatin1("%1-%2%3")
                            .arg(fileName.left(extensionIndex))
                            .arg(index)
                            .arg(fileName.right(fileName.size() - extensionIndex));
                    const QRect rect = QRect(column * tileWidth, row * tileHeight, tileWidth, tileHeight);
                    QImage tileImage = createSubImage(&image, rect);
                    tileImage.save(tileFileName);
                }
            }
        }
    } else {
        image.save(fileName);
        mPath = QFileInfo(fileName).path();
    }

    // Store settings for next time
    QSettings *s = Preferences::instance()->settings();
    s->setValue(QLatin1String(VISIBLE_ONLY_KEY), visibleLayersOnly);
    s->setValue(QLatin1String(CURRENT_SCALE_KEY), useCurrentScale);
    s->setValue(QLatin1String(DRAW_GRID_KEY), drawTileGrid);
    s->setValue(QLatin1String(INCLUDE_BACKGROUND_COLOR), includeBackgroundColor);
    s->setValue(QLatin1String(SPLIT_INTO_TILES_KEY), splitIntoTiles);

    QDialog::accept();
}

void ExportAsImageDialog::browse()
{
    // Don't confirm overwrite here, since we'll confirm when the user presses
    // the Export button
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

void ExportAsImageDialog::updateAcceptEnabled()
{
    QPushButton *saveButton = mUi->buttonBox->button(QDialogButtonBox::Save);
    saveButton->setEnabled(!mUi->fileNameEdit->text().isEmpty());
}
