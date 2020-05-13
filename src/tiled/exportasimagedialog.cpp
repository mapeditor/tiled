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

#include "imagelayer.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "minimaprenderer.h"
#include "objectgroup.h"
#include "objectselectionitem.h"
#include "preferences.h"
#include "session.h"
#include "tilelayer.h"
#include "utils.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QImageWriter>

using namespace Tiled;

namespace session {
static SessionOption<bool> visibleLayersOnly { "exportAsImage.visibleLayersOnly", true };
static SessionOption<bool> useCurrentScale { "exportAsImage.useCurrentScale", true };
static SessionOption<bool> drawTileGrid { "exportAsImage.drawTileGrid", false };
static SessionOption<bool> drawObjectLabels { "exportAsImage.drawObjectLabels", false };
static SessionOption<bool> includeBackgroundColor { "exportAsImage.includeBackgroundColor", false };
} // namespace session

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
    resize(Utils::dpiScaled(size()));
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif

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

    mUi->visibleLayersOnly->setChecked(session::visibleLayersOnly);
    mUi->currentZoomLevel->setChecked(session::useCurrentScale);
    mUi->drawTileGrid->setChecked(session::drawTileGrid);
    mUi->drawObjectLabels->setChecked(session::drawObjectLabels);
    mUi->includeBackgroundColor->setChecked(session::includeBackgroundColor);

    connect(mUi->browseButton, &QAbstractButton::clicked, this, &ExportAsImageDialog::browse);
    connect(mUi->fileNameEdit, &QLineEdit::textChanged,
            this, &ExportAsImageDialog::updateAcceptEnabled);


    Utils::restoreGeometry(this);
}

ExportAsImageDialog::~ExportAsImageDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

static bool smoothTransform(qreal scale)
{
    return scale != qreal(1) && scale < qreal(2);
}

void ExportAsImageDialog::accept()
{
    const QString fileName = mUi->fileNameEdit->text();
    if (fileName.isEmpty())
        return;

    if (QFile::exists(fileName)) {
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

    session::visibleLayersOnly = mUi->visibleLayersOnly->isChecked();
    session::useCurrentScale = mUi->currentZoomLevel->isChecked();
    session::drawTileGrid = mUi->drawTileGrid->isChecked();
    session::drawObjectLabels = mUi->drawObjectLabels->isChecked();
    session::includeBackgroundColor = mUi->includeBackgroundColor->isChecked();

    MiniMapRenderer miniMapRenderer(mMapDocument->map());
    miniMapRenderer.setGridColor(Preferences::instance()->gridColor());

    if (session::drawObjectLabels) {
        miniMapRenderer.setRenderObjectLabelCallback([] (QPainter &painter, const MapObject *object, const MapRenderer &renderer) {
            if (object->name().isEmpty())
                return;

            MapObjectLabel label { object };
            label.syncWithMapObject(renderer);

            const auto invertScale = 1 / renderer.painterScale();
            painter.save();
            painter.translate(label.pos());
            painter.scale(invertScale, invertScale);

            label.paint(&painter, nullptr, nullptr);

            painter.restore();
        });
    }

    MiniMapRenderer::RenderFlags renderFlags(MiniMapRenderer::DrawTileLayers |
                                             MiniMapRenderer::DrawMapObjects |
                                             MiniMapRenderer::DrawImageLayers);

    if (session::visibleLayersOnly)
        renderFlags |= MiniMapRenderer::IgnoreInvisibleLayer;
    if (session::drawTileGrid)
        renderFlags |= MiniMapRenderer::DrawGrid;
    if (session::includeBackgroundColor)
        renderFlags |= MiniMapRenderer::DrawBackground;
    if (session::useCurrentScale && smoothTransform(mCurrentScale))
        renderFlags |= MiniMapRenderer::SmoothPixmapTransform;

    MapRenderer *renderer = mMapDocument->renderer();

    QSize imageSize = renderer->mapBoundingRect().size();

    QMargins margins = mMapDocument->map()->computeLayerOffsetMargins();
    imageSize.setWidth(imageSize.width() + margins.left() + margins.right());
    imageSize.setHeight(imageSize.height() + margins.top() + margins.bottom());

    if (session::useCurrentScale)
        imageSize *= mCurrentScale;

    try {
        QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);

        if (image.isNull()) {
            const size_t gigabyte = 1073741824;
            const size_t memory = size_t(imageSize.width()) * size_t(imageSize.height()) * 4;
            const double gigabytes = static_cast<double>(memory) / gigabyte;

            QMessageBox::critical(this,
                                  tr("Image too Big"),
                                  tr("The resulting image would be %1 x %2 pixels and take %3 GB of memory. "
                                     "Tiled is unable to create such an image. Try reducing the zoom level.")
                                  .arg(imageSize.width())
                                  .arg(imageSize.height())
                                  .arg(gigabytes, 0, 'f', 2));
            return;
        }

        miniMapRenderer.renderToImage(image, renderFlags);

        image.save(fileName);

    } catch (const std::bad_alloc &) {
        QMessageBox::critical(this,
                              tr("Out of Memory"),
                              tr("Could not allocate sufficient memory for the image. "
                                 "Try reducing the zoom level or using a 64-bit version of Tiled."));
        return;
    }

    mPath = QFileInfo(fileName).path();

    QDialog::accept();
}

void ExportAsImageDialog::browse()
{
    // Don't confirm overwrite here, since we'll confirm when the user presses
    // the Export button
    const QString filter = Utils::writableImageFormatsFilter();
    QString f = QFileDialog::getSaveFileName(this, tr("Image"),
                                             mUi->fileNameEdit->text(),
                                             filter, nullptr,
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
