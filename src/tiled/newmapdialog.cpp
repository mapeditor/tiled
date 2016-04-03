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

#include "isometricrenderer.h"
#include "hexagonalrenderer.h"
#include "map.h"
#include "mapdocument.h"
#include "orthogonalrenderer.h"
#include "preferences.h"
#include "staggeredrenderer.h"
#include "tilelayer.h"

#include <QSettings>
#include <QMessageBox>

static const char * const ORIENTATION_KEY = "Map/Orientation";
static const char * const MAP_WIDTH_KEY = "Map/Width";
static const char * const MAP_HEIGHT_KEY = "Map/Height";
static const char * const TILE_WIDTH_KEY = "Map/TileWidth";
static const char * const TILE_HEIGHT_KEY = "Map/TileHeight";

using namespace Tiled;
using namespace Tiled::Internal;

template<typename Type>
static Type comboBoxValue(QComboBox *comboBox)
{
#if QT_VERSION >= 0x050200
    const QVariant currentData = comboBox->currentData();
#else
    const int currentIndex = comboBox->currentIndex();
    const QVariant currentData = comboBox->itemData(currentIndex);
#endif
    return currentData.value<Type>();
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
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Restore previously used settings
    Preferences *prefs = Preferences::instance();
    QSettings *s = prefs->settings();
    const auto orientation = static_cast<Map::Orientation>(s->value(QLatin1String(ORIENTATION_KEY)).toInt());
    const int mapWidth = s->value(QLatin1String(MAP_WIDTH_KEY), 100).toInt();
    const int mapHeight = s->value(QLatin1String(MAP_HEIGHT_KEY), 100).toInt();
    const int tileWidth = s->value(QLatin1String(TILE_WIDTH_KEY), 32).toInt();
    const int tileHeight = s->value(QLatin1String(TILE_HEIGHT_KEY), 32).toInt();

    mUi->layerFormat->addItem(QCoreApplication::translate("PreferencesDialog", "CSV"), QVariant::fromValue(Map::CSV));
    mUi->layerFormat->addItem(QCoreApplication::translate("PreferencesDialog", "Base64 (uncompressed)"), QVariant::fromValue(Map::Base64));
    mUi->layerFormat->addItem(QCoreApplication::translate("PreferencesDialog", "Base64 (zlib compressed)"), QVariant::fromValue(Map::Base64Zlib));

    mUi->renderOrder->addItem(QCoreApplication::translate("PreferencesDialog", "Right Down"), QVariant::fromValue(Map::RightDown));
    mUi->renderOrder->addItem(QCoreApplication::translate("PreferencesDialog", "Right Up"), QVariant::fromValue(Map::RightUp));
    mUi->renderOrder->addItem(QCoreApplication::translate("PreferencesDialog", "Left Down"), QVariant::fromValue(Map::LeftDown));
    mUi->renderOrder->addItem(QCoreApplication::translate("PreferencesDialog", "Left Up"), QVariant::fromValue(Map::LeftUp));

    mUi->orientation->addItem(tr("Orthogonal"), QVariant::fromValue(Map::Orthogonal));
    mUi->orientation->addItem(tr("Isometric"), QVariant::fromValue(Map::Isometric));
    mUi->orientation->addItem(tr("Isometric (Staggered)"), QVariant::fromValue(Map::Staggered));
    mUi->orientation->addItem(tr("Hexagonal (Staggered)"), QVariant::fromValue(Map::Hexagonal));

    if (!setComboBoxValue(mUi->orientation, orientation))
        setComboBoxValue(mUi->orientation, Map::Orthogonal);

    if (!setComboBoxValue(mUi->layerFormat, prefs->layerDataFormat()))
        setComboBoxValue(mUi->layerFormat, Map::CSV);

    setComboBoxValue(mUi->renderOrder, prefs->mapRenderOrder());

    mUi->mapWidth->setValue(mapWidth);
    mUi->mapHeight->setValue(mapHeight);
    mUi->tileWidth->setValue(tileWidth);
    mUi->tileHeight->setValue(tileHeight);

    // Make the font of the pixel size label smaller
    QFont font = mUi->pixelSizeLabel->font();
    const qreal size = QFontInfo(font).pointSizeF();
    font.setPointSizeF(size - 1);
    mUi->pixelSizeLabel->setFont(font);

    connect(mUi->mapWidth, SIGNAL(valueChanged(int)), SLOT(refreshPixelSize()));
    connect(mUi->mapHeight, SIGNAL(valueChanged(int)), SLOT(refreshPixelSize()));
    connect(mUi->tileWidth, SIGNAL(valueChanged(int)), SLOT(refreshPixelSize()));
    connect(mUi->tileHeight, SIGNAL(valueChanged(int)), SLOT(refreshPixelSize()));
    connect(mUi->orientation, SIGNAL(currentIndexChanged(int)), SLOT(refreshPixelSize()));
    refreshPixelSize();
}

NewMapDialog::~NewMapDialog()
{
    delete mUi;
}

MapDocument *NewMapDialog::createMap()
{
    if (exec() != QDialog::Accepted)
        return nullptr;

    const int mapWidth = mUi->mapWidth->value();
    const int mapHeight = mUi->mapHeight->value();
    const int tileWidth = mUi->tileWidth->value();
    const int tileHeight = mUi->tileHeight->value();

    const auto orientation = comboBoxValue<Map::Orientation>(mUi->orientation);
    const auto layerFormat = comboBoxValue<Map::LayerDataFormat>(mUi->layerFormat);
    const auto renderOrder = comboBoxValue<Map::RenderOrder>(mUi->renderOrder);

    Map *map = new Map(orientation,
                       mapWidth, mapHeight,
                       tileWidth, tileHeight);

    map->setLayerDataFormat(layerFormat);
    map->setRenderOrder(renderOrder);

    const size_t gigabyte = 1073741824;
    const size_t memory = size_t(mapWidth) * size_t(mapHeight) * sizeof(Cell);

    // Add a tile layer to new maps of reasonable size
    if (memory < gigabyte) {
        map->addLayer(new TileLayer(tr("Tile Layer 1"), 0, 0,
                                    mapWidth, mapHeight));
    } else {
        const double gigabytes = (double) memory / gigabyte;
        QMessageBox::warning(this, tr("Memory Usage Warning"),
                             tr("Tile layers for this map will consume %L1 GB "
                                "of memory each. Not creating one by default.")
                             .arg(gigabytes, 0, 'f', 2));
    }

    // Store settings for next time
    Preferences *prefs = Preferences::instance();
    prefs->setLayerDataFormat(layerFormat);
    prefs->setMapRenderOrder(renderOrder);
    QSettings *s = Preferences::instance()->settings();
    s->setValue(QLatin1String(ORIENTATION_KEY), orientation);
    s->setValue(QLatin1String(MAP_WIDTH_KEY), mapWidth);
    s->setValue(QLatin1String(MAP_HEIGHT_KEY), mapHeight);
    s->setValue(QLatin1String(TILE_WIDTH_KEY), tileWidth);
    s->setValue(QLatin1String(TILE_HEIGHT_KEY), tileHeight);

    return new MapDocument(map);
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
        size = IsometricRenderer(&map).mapSize();
        break;
    case Map::Staggered:
        size = StaggeredRenderer(&map).mapSize();
        break;
    case Map::Hexagonal:
        size = HexagonalRenderer(&map).mapSize();
        break;
    default:
        size = OrthogonalRenderer(&map).mapSize();
        break;
    }

    mUi->pixelSizeLabel->setText(tr("%1 x %2 pixels")
                                 .arg(size.width())
                                 .arg(size.height()));
}
