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
#include "utils.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

#include <memory>

static const char * const ORIENTATION_KEY = "Map/Orientation";
static const char * const FIXED_SIZE_KEY = "Map/FixedSize";
static const char * const MAP_WIDTH_KEY = "Map/Width";
static const char * const MAP_HEIGHT_KEY = "Map/Height";
static const char * const TILE_WIDTH_KEY = "Map/TileWidth";
static const char * const TILE_HEIGHT_KEY = "Map/TileHeight";

using namespace Tiled;

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

    mUi->fixedSizeSpacer->changeSize(qRound(Utils::dpiScaled(mUi->fixedSizeSpacer->sizeHint().width())), 0,
                                     mUi->fixedSizeSpacer->sizePolicy().horizontalPolicy());

    mUi->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save As..."));

    // Restore previously used settings
    Preferences *prefs = Preferences::instance();
    QSettings *s = prefs->settings();
    const auto orientation = static_cast<Map::Orientation>(s->value(QLatin1String(ORIENTATION_KEY)).toInt());
    const bool fixedSize = s->value(QLatin1String(FIXED_SIZE_KEY), true).toBool();
    const int mapWidth = s->value(QLatin1String(MAP_WIDTH_KEY), 100).toInt();
    const int mapHeight = s->value(QLatin1String(MAP_HEIGHT_KEY), 100).toInt();
    const int tileWidth = s->value(QLatin1String(TILE_WIDTH_KEY), 32).toInt();
    const int tileHeight = s->value(QLatin1String(TILE_HEIGHT_KEY), 32).toInt();

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

    if (!setComboBoxValue(mUi->orientation, orientation))
        setComboBoxValue(mUi->orientation, Map::Orthogonal);

    if (!setComboBoxValue(mUi->layerFormat, prefs->layerDataFormat()))
        setComboBoxValue(mUi->layerFormat, Map::Base64Zstandard);

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

    connect(mUi->mapWidth, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &NewMapDialog::refreshPixelSize);
    connect(mUi->mapHeight, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &NewMapDialog::refreshPixelSize);
    connect(mUi->tileWidth, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &NewMapDialog::refreshPixelSize);
    connect(mUi->tileHeight, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &NewMapDialog::refreshPixelSize);
    connect(mUi->orientation, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &NewMapDialog::refreshPixelSize);
    connect(mUi->fixedSize, &QAbstractButton::toggled, this, &NewMapDialog::updateWidgets);

    if (fixedSize)
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

    const bool fixedSize = mUi->fixedSize->isChecked();
    const int mapWidth = mUi->mapWidth->value();
    const int mapHeight = mUi->mapHeight->value();
    const int tileWidth = mUi->tileWidth->value();
    const int tileHeight = mUi->tileHeight->value();

    const auto orientation = comboBoxValue<Map::Orientation>(mUi->orientation);
    const auto layerFormat = comboBoxValue<Map::LayerDataFormat>(mUi->layerFormat);
    const auto renderOrder = comboBoxValue<Map::RenderOrder>(mUi->renderOrder);

    std::unique_ptr<Map> map { new Map(orientation,
                                       mapWidth, mapHeight,
                                       tileWidth, tileHeight,
                                       !fixedSize) };

    map->setLayerDataFormat(layerFormat);
    map->setRenderOrder(renderOrder);

    const size_t gigabyte = 1073741824u;
    const size_t memory = size_t(mapWidth) * size_t(mapHeight) * sizeof(Cell);

    // Add a tile layer to new maps of reasonable size
    if (memory < gigabyte) {
        map->addLayer(new TileLayer(QCoreApplication::translate("Tiled::MapDocument", "Tile Layer %1").arg(1),
                                    0, 0,
                                    mapWidth, mapHeight));
    } else {
        const double gigabytes = static_cast<double>(memory) / gigabyte;
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
    s->setValue(QLatin1String(FIXED_SIZE_KEY), fixedSize);
    s->setValue(QLatin1String(MAP_WIDTH_KEY), mapWidth);
    s->setValue(QLatin1String(MAP_HEIGHT_KEY), mapHeight);
    s->setValue(QLatin1String(TILE_WIDTH_KEY), tileWidth);
    s->setValue(QLatin1String(TILE_HEIGHT_KEY), tileHeight);

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
