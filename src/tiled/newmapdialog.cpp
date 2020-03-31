/*
 * newmapdialog.cpp
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

#include "newmapdialog.h"
#include "ui_newmapdialog.h"

#include "hexagonalrenderer.h"
#include "isometricrenderer.h"
#include "map.h"
#include "mapdocument.h"
#include "orthogonalrenderer.h"
#include "session.h"
#include "staggeredrenderer.h"
#include "tilelayer.h"
#include "utils.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QPushButton>

#include <memory>

using namespace Tiled;

namespace session {
static SessionOption<Map::Orientation> mapOrientation { "map.orientation", Map::Orthogonal };
static SessionOption<Map::LayerDataFormat> layerDataFormat { "map.layerDataFormat", Map::Base64Zstandard };
static SessionOption<Map::RenderOrder> renderOrder { "map.renderOrder", Map::RightDown };
static SessionOption<bool> fixedSize { "map.fixedSize", true };
static SessionOption<int> mapWidth { "map.width", 100 };
static SessionOption<int> mapHeight { "map.height", 100 };
static SessionOption<int> mapTileWidth { "map.tileWidth", 32 };
static SessionOption<int> mapTileHeight { "map.tileHeight", 32 };
} // namespace session


template<typename Type>
static Type comboBoxValue(QComboBox *comboBox)
{
    return comboBox->currentData().value<Type>();
}

template<typename Type>
static bool setComboBoxValue(QComboBox *comboBox, Type value)
{
    const int index = comboBox->findData(QVariant::fromValue(value));
    if (index == -1)
        return false;
    comboBox->setCurrentIndex(index);
    return true;
}

NewMapDialog::NewMapDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::NewMapDialog)
{
    mUi->setupUi(this);
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif

    mUi->fixedSizeSpacer->changeSize(Utils::dpiScaled(mUi->fixedSizeSpacer->sizeHint().width()), 0,
                                     mUi->fixedSizeSpacer->sizePolicy().horizontalPolicy());

    mUi->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save As..."));

    mUi->layerFormat->addItem(QCoreApplication::translate("PreferencesDialog", "CSV"), QVariant::fromValue(Map::CSV));
    mUi->layerFormat->addItem(QCoreApplication::translate("PreferencesDialog", "Base64 (uncompressed)"), QVariant::fromValue(Map::Base64));
    mUi->layerFormat->addItem(QCoreApplication::translate("PreferencesDialog", "Base64 (zlib compressed)"), QVariant::fromValue(Map::Base64Zlib));
#ifdef TILED_ZSTD_SUPPORT
    mUi->layerFormat->addItem(QCoreApplication::translate("PreferencesDialog", "Base64 (Zstandard compressed)"), QVariant::fromValue(Map::Base64Zstandard));
#endif

    mUi->renderOrder->addItem(QCoreApplication::translate("PreferencesDialog", "Right Down"), QVariant::fromValue(Map::RightDown));
    mUi->renderOrder->addItem(QCoreApplication::translate("PreferencesDialog", "Right Up"), QVariant::fromValue(Map::RightUp));
    mUi->renderOrder->addItem(QCoreApplication::translate("PreferencesDialog", "Left Down"), QVariant::fromValue(Map::LeftDown));
    mUi->renderOrder->addItem(QCoreApplication::translate("PreferencesDialog", "Left Up"), QVariant::fromValue(Map::LeftUp));

    mUi->orientation->addItem(tr("Orthogonal"), QVariant::fromValue(Map::Orthogonal));
    mUi->orientation->addItem(tr("Isometric"), QVariant::fromValue(Map::Isometric));
    mUi->orientation->addItem(tr("Isometric (Staggered)"), QVariant::fromValue(Map::Staggered));
    mUi->orientation->addItem(tr("Hexagonal (Staggered)"), QVariant::fromValue(Map::Hexagonal));

    if (!setComboBoxValue<Map::Orientation>(mUi->orientation, session::mapOrientation))
        setComboBoxValue(mUi->orientation, Map::Orthogonal);

    if (!setComboBoxValue<Map::LayerDataFormat>(mUi->layerFormat, session::layerDataFormat))
        setComboBoxValue(mUi->layerFormat, Map::Base64Zstandard);

    setComboBoxValue<Map::RenderOrder>(mUi->renderOrder, session::renderOrder);

    mUi->mapWidth->setValue(session::mapWidth);
    mUi->mapHeight->setValue(session::mapHeight);
    mUi->tileWidth->setValue(session::mapTileWidth);
    mUi->tileHeight->setValue(session::mapTileHeight);

    Session::current().set("Map/SizeTest", QSize(2, 4300));

    // Make the font of the pixel size label smaller
    QFont font = mUi->pixelSizeLabel->font();
    const qreal size = QFontInfo(font).pointSizeF();
    font.setPointSizeF(size - 1);
    mUi->pixelSizeLabel->setFont(font);

    connect(mUi->mapWidth, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &NewMapDialog::refreshPixelSize);
    connect(mUi->mapHeight, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &NewMapDialog::refreshPixelSize);
    connect(mUi->tileWidth, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &NewMapDialog::refreshPixelSize);
    connect(mUi->tileHeight, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &NewMapDialog::refreshPixelSize);
    connect(mUi->orientation, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &NewMapDialog::refreshPixelSize);
    connect(mUi->fixedSize, &QAbstractButton::toggled, this, &NewMapDialog::updateWidgets);

    if (session::fixedSize)
        mUi->fixedSize->setChecked(true);
    else
        mUi->mapInfinite->setChecked(true);

    refreshPixelSize();
}

NewMapDialog::~NewMapDialog()
{
    delete mUi;
}

MapDocumentPtr NewMapDialog::createMap()
{
    if (exec() != QDialog::Accepted)
        return MapDocumentPtr();

    session::mapOrientation = comboBoxValue<Map::Orientation>(mUi->orientation);
    session::layerDataFormat = comboBoxValue<Map::LayerDataFormat>(mUi->layerFormat);
    session::renderOrder = comboBoxValue<Map::RenderOrder>(mUi->renderOrder);
    session::fixedSize = mUi->fixedSize->isChecked();
    session::mapWidth = mUi->mapWidth->value();
    session::mapHeight = mUi->mapHeight->value();
    session::mapTileWidth = mUi->tileWidth->value();
    session::mapTileHeight = mUi->tileHeight->value();

    std::unique_ptr<Map> map { new Map(session::mapOrientation,
                                       session::mapWidth, session::mapHeight,
                                       session::mapTileWidth, session::mapTileHeight,
                                       !session::fixedSize) };

    map->setLayerDataFormat(session::layerDataFormat);
    map->setRenderOrder(session::renderOrder);

    // Try to set a somewhat helpful default for the hex side length
    if (map->orientation() == Map::Hexagonal) {
        if (map->staggerAxis() == Map::StaggerX)
            map->setHexSideLength(map->tileWidth() / 2);
        else
            map->setHexSideLength(map->tileHeight() / 2);
    }

    const size_t gigabyte = 1073741824u;
    const size_t memory = size_t(map->width()) * size_t(map->height()) * sizeof(Cell);

    // Add a tile layer to new maps of reasonable size
    if (memory < gigabyte) {
        map->addLayer(new TileLayer(QCoreApplication::translate("Tiled::MapDocument", "Tile Layer %1").arg(1),
                                    0, 0,
                                    map->width(), map->height()));
    } else {
        const double gigabytes = static_cast<double>(memory) / gigabyte;
        QMessageBox::warning(this, tr("Memory Usage Warning"),
                             tr("Tile layers for this map will consume %L1 GB "
                                "of memory each. Not creating one by default.")
                             .arg(gigabytes, 0, 'f', 2));
    }

    return MapDocumentPtr::create(std::move(map));
}

void NewMapDialog::refreshPixelSize()
{
    const Map map(comboBoxValue<Map::Orientation>(mUi->orientation),
                  mUi->mapWidth->value(),
                  mUi->mapHeight->value(),
                  mUi->tileWidth->value(),
                  mUi->tileHeight->value());

    QSize size;

    switch (map.orientation()) {
    case Map::Isometric:
        size = IsometricRenderer(&map).mapBoundingRect().size();
        break;
    case Map::Staggered:
        size = StaggeredRenderer(&map).mapBoundingRect().size();
        break;
    case Map::Hexagonal:
        size = HexagonalRenderer(&map).mapBoundingRect().size();
        break;
    default:
        size = OrthogonalRenderer(&map).mapBoundingRect().size();
        break;
    }

    mUi->pixelSizeLabel->setText(tr("%1 x %2 pixels")
                                 .arg(size.width())
                                 .arg(size.height()));
}

void NewMapDialog::updateWidgets(bool checked)
{
    mUi->mapHeight->setEnabled(checked);
    mUi->mapWidth->setEnabled(checked);
    mUi->pixelSizeLabel->setEnabled(checked);
    mUi->heightLabel->setEnabled(checked);
    mUi->widthLabel->setEnabled(checked);
}
