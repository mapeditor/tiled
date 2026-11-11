/*
 * aidock.cpp
 *
 * This file is part of Tiled.
 * Licensed under the GNU General Public License v2 or later.
 */

#include "aidock.h"

#include "changeproperties.h"
#include "claudeclient.h"
#include "documentmanager.h"
#include "map.h"
#include "mapdocument.h"
#include "painttilelayer.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetdocument.h"

#include <QBuffer>
#include <QComboBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QUndoStack>
#include <QVBoxLayout>

#include <cmath>

namespace Tiled {

namespace {

struct SizePreset {
    const char *label;
    int width;
    int height;
};

const SizePreset sizePresets[] = {
    { "Small 4:3 (32 x 24)",       32, 24 },
    { "Medium 3:2 (48 x 32)",      48, 32 },
    { "Large 4:3 (64 x 48)",       64, 48 },
    { "Wide 16:9 (64 x 36)",       64, 36 },
    { "Ultra-wide 3:1 (90 x 30)",  90, 30 },
    { "Square 1:1 (40 x 40)",      40, 40 },
    { "Tall 2:3 (32 x 48)",        32, 48 },
};

} // anonymous namespace

AiDock::AiDock(QWidget *parent)
    : QDockWidget(parent)
    , mClient(new ClaudeClient(this))
{
    setObjectName(QLatin1String("aiDock"));
    setWindowTitle(tr("AI Map Generator"));

    mApiKeyEdit = new QLineEdit;
    mApiKeyEdit->setEchoMode(QLineEdit::Password);
    mApiKeyEdit->setPlaceholderText(tr("Claude API key (sk-ant-...)"));
    mApiKeyEdit->setText(ClaudeClient::apiKey());
    connect(mApiKeyEdit, &QLineEdit::editingFinished, this, [this] {
        ClaudeClient::setApiKey(mApiKeyEdit->text().trimmed());
    });

    mModelEdit = new QLineEdit(ClaudeClient::model());
    connect(mModelEdit, &QLineEdit::editingFinished, this, [this] {
        ClaudeClient::setModel(mModelEdit->text().trimmed());
    });

    auto *tabs = new QTabWidget;
    tabs->addTab(createTilesetTab(), tr("1. Tileset"));
    tabs->addTab(createGenerateTab(), tr("2. Generate"));
    tabs->addTab(createVariantsTab(), tr("3. Options"));

    mStatusLabel = new QLabel;
    mStatusLabel->setWordWrap(true);

    auto *form = new QFormLayout;
    form->addRow(tr("API key"), mApiKeyEdit);
    form->addRow(tr("Model"), mModelEdit);

    auto *layout = new QVBoxLayout;
    layout->setContentsMargins(6, 6, 6, 6);
    layout->addLayout(form);
    layout->addWidget(tabs, 1);
    layout->addWidget(mStatusLabel);

    auto *widget = new QWidget;
    widget->setLayout(layout);
    setWidget(widget);

    auto *documentManager = DocumentManager::instance();
    connect(documentManager, &DocumentManager::currentDocumentChanged,
            this, &AiDock::currentDocumentChanged);
    currentDocumentChanged(documentManager->currentDocument());
}

AiDock::~AiDock() = default;

// ---------------------------------------------------------------------------
// UI construction
// ---------------------------------------------------------------------------

QWidget *AiDock::createTilesetTab()
{
    mTilesetCombo = new QComboBox;
    connect(mTilesetCombo, &QComboBox::currentIndexChanged, this, &AiDock::refreshTileTable);

    mTilesetHint = new QLineEdit;
    mTilesetHint->setPlaceholderText(tr("Optional hint, e.g. 'top-down village, 3x3 houses, road turns'"));

    mNameButton = new QPushButton(tr("Name tiles with Claude"));
    connect(mNameButton, &QPushButton::clicked, this, &AiDock::nameTilesWithClaude);

    mTileTable = new QTableWidget(0, 3);
    mTileTable->setHorizontalHeaderLabels({ tr("Tile"), tr("Id"), tr("Name (editable)") });
    mTileTable->horizontalHeader()->setStretchLastSection(true);
    mTileTable->verticalHeader()->hide();
    mTileTable->setIconSize(QSize(32, 32));
    connect(mTileTable, &QTableWidget::itemChanged, this, &AiDock::tileNameEdited);

    mCompositeList = new QListWidget;
    mCompositeList->setMaximumHeight(90);

    auto *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("Import a tileset in Tiled, then let Claude name its tiles "
                                    "(grass, road_turn_ne, house...). You can rename any tile.")));
    layout->addWidget(mTilesetCombo);
    layout->addWidget(mTilesetHint);
    layout->addWidget(mNameButton);
    layout->addWidget(mTileTable, 1);
    layout->addWidget(new QLabel(tr("Multi-tile objects found:")));
    layout->addWidget(mCompositeList);

    auto *widget = new QWidget;
    widget->setLayout(layout);
    return widget;
}

QWidget *AiDock::createGenerateTab()
{
    mMapPrompt = new QPlainTextEdit;
    mMapPrompt->setPlaceholderText(tr("Describe the map: 'A small riverside village with a market square, "
                                      "a forest to the north and a road leaving east.'"));

    mPresetCombo = new QComboBox;
    for (const SizePreset &preset : sizePresets)
        mPresetCombo->addItem(tr(preset.label));
    mPresetCombo->addItem(tr("Custom"));

    mWidthSpin = new QSpinBox;
    mWidthSpin->setRange(8, 128);
    mHeightSpin = new QSpinBox;
    mHeightSpin->setRange(8, 128);

    connect(mPresetCombo, &QComboBox::currentIndexChanged, this, &AiDock::presetChanged);
    auto markCustom = [this] {
        const int custom = mPresetCombo->count() - 1;
        for (int i = 0; i < custom; ++i) {
            if (sizePresets[i].width == mWidthSpin->value() && sizePresets[i].height == mHeightSpin->value()) {
                mPresetCombo->blockSignals(true);
                mPresetCombo->setCurrentIndex(i);
                mPresetCombo->blockSignals(false);
                return;
            }
        }
        mPresetCombo->blockSignals(true);
        mPresetCombo->setCurrentIndex(custom);
        mPresetCombo->blockSignals(false);
    };
    connect(mWidthSpin, &QSpinBox::valueChanged, this, markCustom);
    connect(mHeightSpin, &QSpinBox::valueChanged, this, markCustom);
    presetChanged();

    mGenerateButton = new QPushButton(tr("Generate map"));
    connect(mGenerateButton, &QPushButton::clicked, this, &AiDock::generateMap);

    auto *sizeLayout = new QHBoxLayout;
    sizeLayout->addWidget(mWidthSpin);
    sizeLayout->addWidget(new QLabel(QStringLiteral("x")));
    sizeLayout->addWidget(mHeightSpin);
    sizeLayout->addWidget(new QLabel(tr("tiles")));

    auto *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("Uses the named tiles of the current map's tilesets. "
                                    "The result opens as a new map.")));
    layout->addWidget(mMapPrompt, 1);
    layout->addWidget(new QLabel(tr("Proportion / size")));
    layout->addWidget(mPresetCombo);
    layout->addLayout(sizeLayout);
    layout->addWidget(mGenerateButton);

    auto *widget = new QWidget;
    widget->setLayout(layout);
    return widget;
}

QWidget *AiDock::createVariantsTab()
{
    mRegionLabel = new QLabel(tr("No area chosen. Select an area with the rectangular select tool."));
    mRegionLabel->setWordWrap(true);

    mUseSelectionButton = new QPushButton(tr("Use selected area"));
    connect(mUseSelectionButton, &QPushButton::clicked, this, &AiDock::useSelectedArea);

    mVariantCountSpin = new QSpinBox;
    mVariantCountSpin->setRange(2, 6);
    mVariantCountSpin->setValue(4);

    mVariantPrompt = new QPlainTextEdit;
    mVariantPrompt->setMaximumHeight(70);
    mVariantPrompt->setPlaceholderText(tr("What should this area become? (e.g. 'market square with stalls')"));

    mVariantsButton = new QPushButton(tr("Generate options"));
    connect(mVariantsButton, &QPushButton::clicked, this, &AiDock::generateVariants);

    mVariantList = new QListWidget;
    mVariantList->setViewMode(QListView::IconMode);
    mVariantList->setIconSize(QSize(128, 128));
    mVariantList->setResizeMode(QListView::Adjust);
    mVariantList->setMovement(QListView::Static);
    mVariantList->setWordWrap(true);
    connect(mVariantList, &QListWidget::itemDoubleClicked, this, &AiDock::applyVariant);

    mApplyVariantButton = new QPushButton(tr("Apply selected option"));
    connect(mApplyVariantButton, &QPushButton::clicked, this, &AiDock::applyVariant);

    auto *countLayout = new QHBoxLayout;
    countLayout->addWidget(new QLabel(tr("Number of options")));
    countLayout->addWidget(mVariantCountSpin);
    countLayout->addStretch();

    auto *layout = new QVBoxLayout;
    layout->addWidget(mRegionLabel);
    layout->addWidget(mUseSelectionButton);
    layout->addLayout(countLayout);
    layout->addWidget(mVariantPrompt);
    layout->addWidget(mVariantsButton);
    layout->addWidget(mVariantList, 1);
    layout->addWidget(mApplyVariantButton);

    auto *widget = new QWidget;
    widget->setLayout(layout);
    return widget;
}

// ---------------------------------------------------------------------------
// Document tracking
// ---------------------------------------------------------------------------

MapDocument *AiDock::mapDocument() const
{
    return qobject_cast<MapDocument*>(DocumentManager::instance()->currentDocument());
}

void AiDock::currentDocumentChanged(Document *document)
{
    if (qobject_cast<MapDocument*>(document))
        refreshTilesets();
}

void AiDock::refreshTilesets()
{
    const QString previous = mTilesetCombo->currentText();

    mTilesetCombo->blockSignals(true);
    mTilesetCombo->clear();
    if (MapDocument *doc = mapDocument()) {
        for (const SharedTileset &tileset : doc->map()->tilesets())
            mTilesetCombo->addItem(tileset->name());
    }
    const int index = mTilesetCombo->findText(previous);
    mTilesetCombo->setCurrentIndex(index >= 0 ? index : 0);
    mTilesetCombo->blockSignals(false);

    refreshTileTable();
}

SharedTileset AiDock::currentTileset() const
{
    const MapDocument *doc = mapDocument();
    const int index = mTilesetCombo->currentIndex();
    if (!doc || index < 0 || index >= doc->map()->tilesets().size())
        return {};
    return doc->map()->tilesets().at(index);
}

void AiDock::refreshTileTable()
{
    mUpdatingTable = true;
    mTileTable->setRowCount(0);
    mCompositeList->clear();

    if (const SharedTileset tileset = currentTileset()) {
        const QList<Tile*> &tiles = tileset->tiles();
        mTileTable->setRowCount(tiles.size());

        for (int row = 0; row < tiles.size(); ++row) {
            Tile *tile = tiles.at(row);

            auto *iconItem = new QTableWidgetItem;
            iconItem->setIcon(QIcon(tile->image()));
            iconItem->setFlags(Qt::ItemIsEnabled);

            auto *idItem = new QTableWidgetItem(QString::number(tile->id()));
            idItem->setFlags(Qt::ItemIsEnabled);

            auto *nameItem = new QTableWidgetItem(tile->className());
            nameItem->setData(Qt::UserRole, tile->id());

            mTileTable->setItem(row, 0, iconItem);
            mTileTable->setItem(row, 1, idItem);
            mTileTable->setItem(row, 2, nameItem);
        }

        const auto composites = AiTileCatalog::loadComposites(*tileset);
        for (const AiComposite &composite : composites) {
            mCompositeList->addItem(tr("%1 (%2 x %3 tiles)")
                                    .arg(composite.name).arg(composite.cols).arg(composite.rows));
        }
    }

    mUpdatingTable = false;
}

void AiDock::tileNameEdited(QTableWidgetItem *item)
{
    if (mUpdatingTable || item->column() != 2)
        return;

    const SharedTileset tileset = currentTileset();
    if (!tileset)
        return;

    Tile *tile = tileset->findTile(item->data(Qt::UserRole).toInt());
    TilesetDocument *tilesetDocument = DocumentManager::instance()->findTilesetDocument(tileset);
    if (!tile)
        return;

    const QString name = item->text().trimmed().toLower().replace(QLatin1Char(' '), QLatin1Char('_'));
    if (tilesetDocument)
        tilesetDocument->undoStack()->push(new ChangeClassName(tilesetDocument, { tile }, name));
    else
        tile->setClassName(name);
}

// ---------------------------------------------------------------------------
// Tileset naming
// ---------------------------------------------------------------------------

QImage AiDock::annotatedTilesetImage(const Tileset &tileset) const
{
    const QPixmap &pixmap = tileset.image();
    const int tw = tileset.tileWidth();
    const int th = tileset.tileHeight();
    const int columns = tileset.columnCount();
    if (pixmap.isNull() || columns < 1)
        return {};

    // Upscale small tiles so the ids are readable, but stay within what the
    // vision model handles well.
    qreal scale = qMax(1.0, std::ceil(48.0 / qMin(tw, th)));
    const qreal maxEdge = 1500.0;
    scale = qMin(scale, maxEdge / qMax(pixmap.width(), pixmap.height()));

    QImage image = pixmap.toImage()
            .scaled(pixmap.size() * scale, Qt::KeepAspectRatio, Qt::FastTransformation)
            .convertToFormat(QImage::Format_ARGB32_Premultiplied);

    QPainter painter(&image);
    QFont font = painter.font();
    font.setPixelSize(qBound(9, int(qMin(tw, th) * scale / 3), 16));
    font.setBold(true);
    painter.setFont(font);

    for (int id = 0; id < tileset.tileCount(); ++id) {
        const int col = id % columns;
        const int row = id / columns;
        const QRectF rect((tileset.margin() + col * (tw + tileset.tileSpacing())) * scale,
                          (tileset.margin() + row * (th + tileset.tileSpacing())) * scale,
                          tw * scale, th * scale);

        painter.setPen(QPen(QColor(255, 0, 255, 160), 1));
        painter.drawRect(rect);

        const QString text = QString::number(id);
        const QRectF textRect = rect.adjusted(2, 1, 0, 0);
        painter.setPen(Qt::black);
        painter.drawText(textRect.translated(1, 1), Qt::AlignLeft | Qt::AlignTop, text);
        painter.setPen(Qt::yellow);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, text);
    }
    return image;
}

void AiDock::nameTilesWithClaude()
{
    const SharedTileset tileset = currentTileset();
    if (!tileset) {
        setBusy(false, tr("Open a map with a tileset first."));
        return;
    }

    const QImage image = annotatedTilesetImage(*tileset);
    if (image.isNull()) {
        setBusy(false, tr("Only tilesets based on a single image can be named automatically."));
        return;
    }

    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    const int columns = tileset->columnCount();
    const int rows = (tileset->tileCount() + columns - 1) / columns;

    const QString prompt = tr(
        "This is a tileset image. Tiles are %1x%2 px, in a grid of %3 columns and %4 rows (%5 tiles). "
        "Each tile has its id (row-major, starting at 0) drawn in its top-left corner; the grid is overlaid in magenta.\n\n"
        "Give every non-empty tile a short lowercase snake_case name that describes what it is and how it is used "
        "(for example grass, water, water_edge_top, road_corner_ne, road_straight_h, tree, roof_left, wall_door). "
        "Names must be unique. Leave out fully transparent tiles. For variants use a numeric suffix (grass_2).\n\n"
        "Also list composites: objects made of several adjacent tiles such as houses, trees, bridges or wells. "
        "For each give a name, cols, rows and the tile ids row-major (use -1 for an empty cell).\n\n"
        "%6"
        "Reply with ONE JSON object only:\n"
        "{\"tiles\":[{\"id\":0,\"name\":\"grass\"}],"
        "\"composites\":[{\"name\":\"house\",\"cols\":3,\"rows\":3,\"ids\":[10,11,12,20,21,22,30,31,32]}]}")
        .arg(tileset->tileWidth()).arg(tileset->tileHeight())
        .arg(columns).arg(rows).arg(tileset->tileCount())
        .arg(mTilesetHint->text().trimmed().isEmpty()
             ? QString()
             : tr("Context from the user: %1\n\n").arg(mTilesetHint->text().trimmed()));

    setBusy(true, tr("Claude is looking at the tileset..."));

    QPointer<AiDock> self(this);
    mClient->send(tr("You label tiles of 2D game tilesets precisely. Reply with JSON only."),
                  { ClaudeClient::imageBlock(png), ClaudeClient::textBlock(prompt) },
                  16000,
                  [self, tileset](const ClaudeClient::Result &result) {
        if (!self)
            return;
        if (!result.ok) {
            self->setBusy(false, result.error);
            return;
        }

        QString error;
        const QJsonObject reply = ClaudeClient::extractJson(result.text, &error);
        if (reply.isEmpty()) {
            self->setBusy(false, error);
            return;
        }

        self->applyTilesetLabels(reply, tileset);
    });
}

void AiDock::applyTilesetLabels(const QJsonObject &reply, SharedTileset tileset)
{
    TilesetDocument *tilesetDocument = DocumentManager::instance()->findTilesetDocument(tileset);
    QUndoStack *undoStack = tilesetDocument ? tilesetDocument->undoStack() : nullptr;

    if (undoStack)
        undoStack->beginMacro(tr("Name Tiles with Claude"));

    int named = 0;
    QSet<QString> used;
    const QJsonArray tiles = reply.value(QLatin1String("tiles")).toArray();
    for (const QJsonValue &value : tiles) {
        const QJsonObject object = value.toObject();
        Tile *tile = tileset->findTile(object.value(QLatin1String("id")).toInt(-1));
        QString name = object.value(QLatin1String("name")).toString()
                .trimmed().toLower().replace(QLatin1Char(' '), QLatin1Char('_'));
        if (!tile || name.isEmpty())
            continue;

        // Keep names unique
        QString unique = name;
        for (int n = 2; used.contains(unique); ++n)
            unique = QStringLiteral("%1_%2").arg(name).arg(n);
        used.insert(unique);

        if (undoStack)
            undoStack->push(new ChangeClassName(tilesetDocument, { tile }, unique));
        else
            tile->setClassName(unique);
        ++named;
    }

    // Composites reference tile ids, so they can be stored as they are.
    QVector<AiComposite> composites;
    const QJsonArray compositeArray = reply.value(QLatin1String("composites")).toArray();
    for (const QJsonValue &value : compositeArray) {
        const QJsonObject object = value.toObject();
        AiComposite composite;
        composite.name = object.value(QLatin1String("name")).toString()
                .trimmed().toLower().replace(QLatin1Char(' '), QLatin1Char('_'));
        composite.cols = object.value(QLatin1String("cols")).toInt(1);
        composite.rows = object.value(QLatin1String("rows")).toInt(1);
        const QJsonArray ids = object.value(QLatin1String("ids")).toArray();
        for (const QJsonValue &id : ids)
            composite.tiles.append(id.toInt(-1) >= 0 ? tileset->findTile(id.toInt()) : nullptr);

        if (!composite.name.isEmpty() && composite.cols >= 1 && composite.rows >= 1
                && composite.tiles.size() == composite.cols * composite.rows)
            composites.append(composite);
    }

    const QVariant compositeJson = AiTileCatalog::compositesToJson(composites);
    if (undoStack)
        undoStack->push(new SetProperty(tilesetDocument, { tileset.data() },
                                        QLatin1String(AiTileCatalog::CompositesProperty), compositeJson));
    else
        tileset->setProperty(QLatin1String(AiTileCatalog::CompositesProperty), compositeJson);

    if (undoStack)
        undoStack->endMacro();

    refreshTileTable();
    setBusy(false, tr("Named %1 tiles and found %2 multi-tile objects. Review the names, then generate a map.")
                   .arg(named).arg(composites.size()));
}

// ---------------------------------------------------------------------------
// Map generation
// ---------------------------------------------------------------------------

void AiDock::presetChanged()
{
    const int index = mPresetCombo->currentIndex();
    if (index < 0 || index >= int(std::size(sizePresets)))
        return;   // Custom

    mWidthSpin->blockSignals(true);
    mHeightSpin->blockSignals(true);
    mWidthSpin->setValue(sizePresets[index].width);
    mHeightSpin->setValue(sizePresets[index].height);
    mWidthSpin->blockSignals(false);
    mHeightSpin->blockSignals(false);
}

void AiDock::generateMap()
{
    MapDocument *doc = mapDocument();
    if (!doc) {
        setBusy(false, tr("Open a map that uses your tileset first."));
        return;
    }

    const AiTileCatalog catalog(doc->map()->tilesets());
    if (catalog.isEmpty()) {
        setBusy(false, tr("No named tiles yet. Use the Tileset tab first."));
        return;
    }
    if (mMapPrompt->toPlainText().trimmed().isEmpty()) {
        setBusy(false, tr("Describe the map you want."));
        return;
    }

    const QSize size(mWidthSpin->value(), mHeightSpin->value());
    const QString prompt = AiMapBuilder::generationPrompt(catalog, mMapPrompt->toPlainText().trimmed(), size);
    const int maxTokens = qMin(32000, 3000 + size.width() * size.height() * 4);

    setBusy(true, tr("Claude is designing the map..."));

    // Keep a copy of the tileset list so the reply can be built even if the
    // user switches documents while waiting.
    QPointer<AiDock> self(this);
    QPointer<MapDocument> sourceDoc(doc);
    const QVector<SharedTileset> tilesets = doc->map()->tilesets();
    const Map::Parameters parameters = doc->map()->parameters();

    mClient->send(AiMapBuilder::systemPrompt(),
                  { ClaudeClient::textBlock(prompt) },
                  maxTokens,
                  [self, tilesets, parameters, size](const ClaudeClient::Result &result) {
        if (!self)
            return;
        if (!result.ok) {
            self->setBusy(false, result.error);
            return;
        }

        QString error;
        const QJsonObject reply = ClaudeClient::extractJson(result.text, &error);
        if (reply.isEmpty()) {
            self->setBusy(false, error);
            return;
        }

        const AiTileCatalog catalog(tilesets);
        const AiMapBuilder builder(catalog);

        Map templateMap(parameters);
        for (const SharedTileset &tileset : tilesets)
            templateMap.addTileset(tileset);

        QStringList warnings;
        std::unique_ptr<Map> map = builder.buildMap(reply, templateMap, size, &warnings);
        if (map->layerCount() == 0) {
            self->setBusy(false, tr("Claude's reply contained no layers."));
            return;
        }

        DocumentManager::instance()->addDocument(DocumentPtr(new MapDocument(std::move(map))));
        self->setBusy(false, tr("Map generated (%1 x %2).").arg(size.width()).arg(size.height()));
        self->showWarnings(warnings);
    });
}

// ---------------------------------------------------------------------------
// Options for an area
// ---------------------------------------------------------------------------

void AiDock::useSelectedArea()
{
    MapDocument *doc = mapDocument();
    if (!doc) {
        setBusy(false, tr("Open a map first."));
        return;
    }

    const QRect area = doc->selectedArea().boundingRect();
    if (area.isEmpty()) {
        setBusy(false, tr("Nothing selected. Use the rectangular select tool on the map."));
        return;
    }
    if (area.width() > 32 || area.height() > 32) {
        setBusy(false, tr("Please select an area of at most 32 x 32 tiles."));
        return;
    }

    mRegion = area;
    mVariantDocument = doc;
    mRegionLabel->setText(tr("Area: x=%1 y=%2, %3 x %4 tiles")
                          .arg(area.x()).arg(area.y()).arg(area.width()).arg(area.height()));
    setBusy(false, QString());
}

void AiDock::generateVariants()
{
    MapDocument *doc = mVariantDocument ? mVariantDocument.data() : mapDocument();
    if (!doc || mRegion.isEmpty()) {
        setBusy(false, tr("Choose an area first."));
        return;
    }
    if (doc != mapDocument()) {
        setBusy(false, tr("The chosen area belongs to another map. Switch back or choose again."));
        return;
    }

    const AiTileCatalog catalog(doc->map()->tilesets());
    if (catalog.isEmpty()) {
        setBusy(false, tr("No named tiles yet. Use the Tileset tab first."));
        return;
    }

    const QRect region = mRegion.intersected(QRect(0, 0, doc->map()->width(), doc->map()->height()));
    const int count = mVariantCountSpin->value();
    const QString prompt = AiMapBuilder::variantsPrompt(catalog, *doc->map(), region,
                                                        mVariantPrompt->toPlainText(), count);
    const int maxTokens = qMin(32000, 3000 + count * region.width() * region.height() * 4 * qMax(1, doc->map()->tileLayerCount()));

    setBusy(true, tr("Claude is designing %1 options...").arg(count));

    QPointer<AiDock> self(this);
    QPointer<MapDocument> docPointer(doc);
    const QVector<SharedTileset> tilesets = doc->map()->tilesets();
    const Map::Parameters parameters = doc->map()->parameters();

    mClient->send(AiMapBuilder::systemPrompt(),
                  { ClaudeClient::textBlock(prompt) },
                  maxTokens,
                  [self, docPointer, tilesets, parameters, region](const ClaudeClient::Result &result) {
        if (!self)
            return;
        if (!result.ok) {
            self->setBusy(false, result.error);
            return;
        }

        QString error;
        const QJsonObject reply = ClaudeClient::extractJson(result.text, &error);
        if (reply.isEmpty()) {
            self->setBusy(false, error);
            return;
        }

        const AiTileCatalog catalog(tilesets);
        const AiMapBuilder builder(catalog);

        Map sourceMap(parameters);
        for (const SharedTileset &tileset : tilesets)
            sourceMap.addTileset(tileset);

        QStringList warnings;
        self->mVariants = builder.buildVariants(reply, sourceMap, region.size(), &warnings);
        self->mVariantList->clear();
        for (const AiVariant &variant : self->mVariants) {
            auto *item = new QListWidgetItem(QIcon(QPixmap::fromImage(variant.preview)), variant.label);
            self->mVariantList->addItem(item);
        }
        if (self->mVariantList->count() > 0)
            self->mVariantList->setCurrentRow(0);

        self->setBusy(false, self->mVariants.empty()
                      ? tr("Claude returned no options.")
                      : tr("%1 options ready. Double-click or press Apply to use one.").arg(self->mVariants.size()));
        self->showWarnings(warnings);
    });
}

void AiDock::applyVariant()
{
    const int row = mVariantList->currentRow();
    MapDocument *doc = mVariantDocument.data();
    if (row < 0 || row >= int(mVariants.size()) || !doc || doc != mapDocument()) {
        setBusy(false, tr("Select an option for the current map first."));
        return;
    }

    const AiVariant &variant = mVariants[row];
    const QRect region = mRegion.intersected(QRect(0, 0, doc->map()->width(), doc->map()->height()));

    doc->undoStack()->beginMacro(tr("Apply AI Option '%1'").arg(variant.label));
    int applied = 0;
    for (Layer *layer : variant.map->tileLayers()) {
        auto *source = static_cast<TileLayer*>(layer);

        TileLayer *target = nullptr;
        for (Layer *candidate : doc->map()->tileLayers()) {
            if (candidate->name() == layer->name()) {
                target = static_cast<TileLayer*>(candidate);
                break;
            }
        }
        if (!target)
            continue;

        doc->undoStack()->push(new PaintTileLayer(doc, target, region.x(), region.y(),
                                                  source, QRegion(region)));
        ++applied;
    }
    doc->undoStack()->endMacro();

    setBusy(false, applied > 0
            ? tr("Applied '%1'. Undo restores the previous area.").arg(variant.label)
            : tr("The option had no layers matching this map's layer names."));
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void AiDock::setBusy(bool busy, const QString &status)
{
    mBusy = busy;
    for (QPushButton *button : { mNameButton, mGenerateButton, mVariantsButton, mApplyVariantButton, mUseSelectionButton })
        button->setEnabled(!busy);
    mStatusLabel->setText(status);
}

void AiDock::showWarnings(const QStringList &warnings)
{
    if (warnings.isEmpty())
        return;
    mStatusLabel->setText(mStatusLabel->text()
                          + tr("\n%1 minor issues, e.g. %2").arg(warnings.size()).arg(warnings.first()));
}

} // namespace Tiled

#include "moc_aidock.cpp"
