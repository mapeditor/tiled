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
#include "pluginmanager.h"
#include "mapreaderinterface.h"
#include "rtbmapsettings.h"

#include <QSettings>
#include <QMessageBox>
#include <QPushButton>

static const char * const ORIENTATION_KEY = "Map/Orientation";
static const char * const MAP_WIDTH_KEY = "Map/Width";
static const char * const MAP_HEIGHT_KEY = "Map/Height";
static const char * const TILE_WIDTH_KEY = "Map/TileWidth";
static const char * const TILE_HEIGHT_KEY = "Map/TileHeight";
static const char * const ADD_STARTER_CONTENT_KEY = "Map/AddStarterContent";

using namespace Tiled;
using namespace Tiled::Internal;

NewMapDialog::NewMapDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::NewMapDialog)
{
    mUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);


    // Restore previously used settings
    Preferences *prefs = Preferences::instance();
    QSettings *s = prefs->settings();
    //const int orientation = s->value(QLatin1String(ORIENTATION_KEY)).toInt();
    const int mapWidth = s->value(QLatin1String(MAP_WIDTH_KEY), 100).toInt();
    const int mapHeight = s->value(QLatin1String(MAP_HEIGHT_KEY), 100).toInt();
    //const int tileWidth = s->value(QLatin1String(TILE_WIDTH_KEY), 32).toInt();
    //const int tileHeight = s->value(QLatin1String(TILE_HEIGHT_KEY), 32).toInt();
    const bool addStarterContent = s->value(QLatin1String(ADD_STARTER_CONTENT_KEY), true).toBool();

    mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    mUi->levelName->setText(QLatin1String(""));
    mUi->levelName->setFocus();
    connect(mUi->levelName, SIGNAL(textChanged(QString)), SLOT(enableButton()));

    mUi->addStarterContent->setChecked(addStarterContent);
    mUi->mapWidth->setValue(mapWidth);
    mUi->mapHeight->setValue(mapHeight);
    connect(mUi->mapWidth, SIGNAL(valueChanged(int)), SLOT(refreshPixelSize()));
    connect(mUi->mapHeight, SIGNAL(valueChanged(int)), SLOT(refreshPixelSize()));

    // width and height musst be even
    connect(mUi->mapWidth, SIGNAL(valueChanged(int)), mUi->mapHeight, SLOT(setValue(int)));
    connect(mUi->mapHeight, SIGNAL(valueChanged(int)), mUi->mapWidth, SLOT(setValue(int)));

    refreshPixelSize();

    //QStringList difficultyNames({tr("None"), tr("Easy"), tr("Medium"), tr("Hard"), tr("Extrem")});
    QStringList difficultyNames(QList<QString>() << tr("Choose...") << tr("Easy") << tr("Medium") << tr("Hard") << tr("Extrem"));
    mUi->difficulty->addItems(difficultyNames);
    connect(mUi->difficulty, SIGNAL(currentIndexChanged(int)), SLOT(enableButton()));

    //QStringList playStyleNames({tr("None"), tr("Speed"), tr("Puzzle"), tr("Rhythm"), tr("Mix")});
    QStringList playStyleNames(QList<QString>() << tr("Choose...") << tr("Speed") << tr("Puzzle") << tr("Rhythm") << tr("Mix"));
    mUi->playStyle->addItems(playStyleNames);
    connect(mUi->playStyle, SIGNAL(currentIndexChanged(int)), SLOT(enableButton()));
}

NewMapDialog::~NewMapDialog()
{
    delete mUi;
}

MapDocument *NewMapDialog::createMap()
{
    if (exec() != QDialog::Accepted)
        return 0;

    const int mapWidth = mUi->mapWidth->value();
    const int mapHeight = mUi->mapHeight->value();

    const int tileWidth = 32;
    const int tileHeight = 32;

    const Map::Orientation orientation =
            static_cast<Map::Orientation>(1);
    const Map::LayerDataFormat layerFormat =
            static_cast<Map::LayerDataFormat>(4);
    const Map::RenderOrder renderOrder =
            static_cast<Map::RenderOrder>(0);

    Map *map = new Map(orientation,
                       mapWidth, mapHeight,
                       tileWidth, tileHeight);

    map->setLayerDataFormat(layerFormat);
    map->setRenderOrder(renderOrder);

    RTBMap *rtbMap = map->rtbMap();
    rtbMap->setLevelName(mUi->levelName->text());
    rtbMap->setDifficulty(mUi->difficulty->currentIndex());
    rtbMap->setPlayStyle(mUi->playStyle->currentIndex());

    MapDocument *mapDocument = new MapDocument(map);

    RTBMapSettings *rtbMapSettings = new RTBMapSettings;
    rtbMapSettings->loadTileSets(mapDocument);
    rtbMapSettings->setMapSettings(map);

    // add the starter content to the new map else create the layers
    if(mUi->addStarterContent->isChecked())
        rtbMapSettings->addStarterContent(mapDocument);
    else
        rtbMapSettings->createLayers(mapDocument);

    // set the floor layer selected
    mapDocument->setCurrentLayerIndex(RTBMapSettings::FloorID);

    // show map properties
    mapDocument->setCurrentObject(map);
    mapDocument->emitEditCurrentObject();

    // Store settings for next time
    QSettings *s = Preferences::instance()->settings();
    s->setValue(QLatin1String(MAP_WIDTH_KEY), mapWidth);
    s->setValue(QLatin1String(MAP_HEIGHT_KEY), mapHeight);
    s->setValue(QLatin1String(ADD_STARTER_CONTENT_KEY), mUi->addStarterContent->isChecked());

    return mapDocument;
}

void NewMapDialog::refreshPixelSize()
{
    //const int orientationIndex = mUi->orientation->currentIndex();
    //const QVariant orientationData = mUi->orientation->itemData(orientationIndex);
    const Map::Orientation orientation =
            static_cast<Map::Orientation>(1);

    const Map map(orientation,
                  mUi->mapWidth->value(),
                  mUi->mapHeight->value(),
                  32,
                  32);

    QSize size;

    switch (orientation) {
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

void NewMapDialog::enableButton()
{
    QPushButton *button = mUi->buttonBox->button(QDialogButtonBox::Ok);
    QString text = mUi->levelName->text();
    int difficulty = mUi->difficulty->currentIndex();
    int playStyle = mUi->playStyle->currentIndex();

    if(text.length() < 5 || text.isEmpty() || difficulty == 0 || playStyle == 0)
        button->setEnabled(false);
    else
        button->setEnabled(true);
}
