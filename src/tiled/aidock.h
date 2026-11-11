/*
 * aidock.h
 *
 * Dock widget with Claude-powered tools: tileset naming, map generation and
 * regenerating a selected area with several options.
 *
 * This file is part of Tiled.
 * Licensed under the GNU General Public License v2 or later.
 */

#pragma once

#include "aimapbuilder.h"

#include <QDockWidget>
#include <QPointer>
#include <QRect>

#include <vector>

class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTableWidgetItem;

namespace Tiled {

class ClaudeClient;
class Document;
class MapDocument;

class AiDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit AiDock(QWidget *parent = nullptr);
    ~AiDock() override;

private:
    QWidget *createTilesetTab();
    QWidget *createGenerateTab();
    QWidget *createVariantsTab();

    MapDocument *mapDocument() const;
    void currentDocumentChanged(Document *document);
    void refreshTilesets();
    void refreshTileTable();
    void tileNameEdited(QTableWidgetItem *item);

    void nameTilesWithClaude();
    void applyTilesetLabels(const QJsonObject &reply, SharedTileset tileset);

    void presetChanged();
    void generateMap();

    void useSelectedArea();
    void generateVariants();
    void applyVariant();

    void setBusy(bool busy, const QString &status = QString());
    void showWarnings(const QStringList &warnings);
    SharedTileset currentTileset() const;
    QImage annotatedTilesetImage(const Tileset &tileset) const;

    ClaudeClient *mClient;
    bool mBusy = false;

    QLineEdit *mApiKeyEdit;
    QLineEdit *mModelEdit;
    QLabel *mStatusLabel;

    // Tileset tab
    QComboBox *mTilesetCombo;
    QLineEdit *mTilesetHint;
    QPushButton *mNameButton;
    QTableWidget *mTileTable;
    QListWidget *mCompositeList;
    bool mUpdatingTable = false;

    // Generate tab
    QPlainTextEdit *mMapPrompt;
    QComboBox *mPresetCombo;
    QSpinBox *mWidthSpin;
    QSpinBox *mHeightSpin;
    QPushButton *mGenerateButton;

    // Variants tab
    QLabel *mRegionLabel;
    QPushButton *mUseSelectionButton;
    QSpinBox *mVariantCountSpin;
    QPlainTextEdit *mVariantPrompt;
    QPushButton *mVariantsButton;
    QListWidget *mVariantList;
    QPushButton *mApplyVariantButton;

    QRect mRegion;
    std::vector<AiVariant> mVariants;
    QPointer<MapDocument> mVariantDocument;
};

} // namespace Tiled
