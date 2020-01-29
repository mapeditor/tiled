/*
 * newtilesetdialog.cpp
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

#include "newtilesetdialog.h"
#include "ui_newtilesetdialog.h"

#include "documentmanager.h"
#include "imagecolorpickerwidget.h"
#include "mapdocument.h"
#include "preferences.h"
#include "utils.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QMessageBox>
#include <QSettings>
#include <QCheckBox>

static const char * const TYPE_KEY = "Tileset/Type";
static const char * const EMBED_KEY = "Tileset/EmbedInMap";
static const char * const COLOR_ENABLED_KEY = "Tileset/UseTransparentColor";
static const char * const COLOR_KEY = "Tileset/TransparentColor";
static const char * const TILE_SIZE_KEY = "Tileset/TileSize";
static const char * const SPACING_KEY = "Tileset/Spacing";
static const char * const MARGIN_KEY = "Tileset/Margin";

using namespace Tiled;

enum TilesetType {
    TilesetImage,
    ImageCollection
};

static TilesetType tilesetType(Ui::NewTilesetDialog *ui)
{
    switch (ui->tilesetType->currentIndex()) {
    default:
    case 0:
        return TilesetImage;
    case 1:
        return ImageCollection;
    }
}

NewTilesetDialog::NewTilesetDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::NewTilesetDialog),
    mNameWasEdited(false)
{
    mUi->setupUi(this);
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif

    // Restore previously used settings
    QSettings *s = Preferences::instance()->settings();

    int tilesetType = s->value(QLatin1String(TYPE_KEY)).toInt();
    bool embedded = s->value(QLatin1String(EMBED_KEY)).toBool();
    bool colorEnabled = s->value(QLatin1String(COLOR_ENABLED_KEY)).toBool();
    QString colorName = s->value(QLatin1String(COLOR_KEY)).toString();
    QColor color = QColor::isValidColor(colorName) ? QColor(colorName) : Qt::magenta;
    QSize tileSize = s->value(QLatin1String(TILE_SIZE_KEY)).toSize();
    int spacing = s->value(QLatin1String(SPACING_KEY)).toInt();
    int margin = s->value(QLatin1String(MARGIN_KEY)).toInt();

    mUi->tilesetType->setCurrentIndex(tilesetType);
    mUi->embedded->setChecked(embedded);
    mUi->useTransparentColor->setChecked(colorEnabled);
    mUi->colorButton->setColor(color);
    if (tileSize.isValid()) {
        mUi->tileWidth->setValue(tileSize.width());
        mUi->tileHeight->setValue(tileSize.height());
    }
    mUi->spacing->setValue(spacing);
    mUi->margin->setValue(margin);

    connect(mUi->browseButton, &QAbstractButton::clicked, this, &NewTilesetDialog::browse);
    connect(mUi->name, &QLineEdit::textEdited, this, &NewTilesetDialog::nameEdited);
    connect(mUi->name, &QLineEdit::textChanged, this, &NewTilesetDialog::updateOkButton);
    connect(mUi->embedded, &QCheckBox::toggled, this, &NewTilesetDialog::updateOkButton);
    connect(mUi->image, &QLineEdit::textChanged, this, &NewTilesetDialog::updateOkButton);
    connect(mUi->image, &QLineEdit::textChanged, this, &NewTilesetDialog::updateColorPickerButton);
    connect(mUi->useTransparentColor, &QCheckBox::toggled, this, &NewTilesetDialog::updateColorPickerButton);
    connect(mUi->tilesetType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &NewTilesetDialog::tilesetTypeChanged);
    connect(mUi->dropperButton, &QAbstractButton::clicked, this, &NewTilesetDialog::pickColorFromImage);

    connect(mUi->buttonBox, &QDialogButtonBox::accepted, this, &NewTilesetDialog::tryAccept);
    connect(mUi->buttonBox, &QDialogButtonBox::rejected, this, &NewTilesetDialog::reject);

    mUi->imageGroupBox->setVisible(tilesetType == 0);
    updateOkButton();
}

NewTilesetDialog::~NewTilesetDialog()
{
    delete mUi;
}

/**
 * Sets the path to start in by default.
 *
 * Also sets the image and name fields if the given path is a file.
 */
void NewTilesetDialog::setImagePath(const QString &path)
{
    mPath = path;

    const QFileInfo fileInfo(path);
    if (fileInfo.isFile()) {
        mUi->image->setText(path);
        mUi->name->setText(fileInfo.completeBaseName());
    }
}

void NewTilesetDialog::setTileSize(QSize size)
{
    mUi->tileWidth->setValue(size.width());
    mUi->tileHeight->setValue(size.height());
}

/**
 * Shows the dialog and returns the created tileset. Returns 0 if the
 * dialog was cancelled.
 */
SharedTileset NewTilesetDialog::createTileset()
{
    bool couldEmbed = false;
    if (auto document = DocumentManager::instance()->currentDocument()) {
        if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
            couldEmbed = true;
            setTileSize(mapDocument->map()->tileSize());
        }
    }

    mUi->embedded->setEnabled(couldEmbed);

    setMode(CreateTileset);

    if (exec() != QDialog::Accepted)
        return SharedTileset();

    return mNewTileset;
}

bool NewTilesetDialog::isEmbedded() const
{
    return mUi->embedded->isChecked();
}

/**
 * Shows the dialog and allows to change the given parameters.
 *
 * Returns whether the dialog was accepted.
 */
bool NewTilesetDialog::editTilesetParameters(TilesetParameters &parameters)
{
    setMode(EditTilesetParameters);

    // todo: support remote files
    mPath = parameters.imageSource.toLocalFile();
    mUi->image->setText(parameters.imageSource.toString(QUrl::PreferLocalFile));

    QColor transparentColor = parameters.transparentColor;
    mUi->useTransparentColor->setChecked(transparentColor.isValid());
    if (transparentColor.isValid())
        mUi->colorButton->setColor(transparentColor);

    mUi->tileWidth->setValue(parameters.tileSize.width());
    mUi->tileHeight->setValue(parameters.tileSize.height());
    mUi->spacing->setValue(parameters.tileSpacing);
    mUi->margin->setValue(parameters.margin);

    if (exec() != QDialog::Accepted)
        return false;

    parameters = TilesetParameters(*mNewTileset);
    return true;
}

void NewTilesetDialog::tryAccept()
{
    // Used for storing the settings for next time
    QSettings *s = Preferences::instance()->settings();

    const QString name = mUi->name->text();

    SharedTileset tileset;

    if (tilesetType(mUi) == TilesetImage) {
        const QString image = mUi->image->text();
        const bool useTransparentColor = mUi->useTransparentColor->isChecked();
        const QColor transparentColor = mUi->colorButton->color();
        const int tileWidth = mUi->tileWidth->value();
        const int tileHeight = mUi->tileHeight->value();
        const int spacing = mUi->spacing->value();
        const int margin = mUi->margin->value();

        tileset = Tileset::create(name,
                                  tileWidth, tileHeight,
                                  spacing, margin);

        if (useTransparentColor)
            tileset->setTransparentColor(transparentColor);

        if (!image.isEmpty()) {
            if (!tileset->loadFromImage(image)) {
                QMessageBox::critical(this, tr("Error"),
                                      tr("Failed to load tileset image '%1'.")
                                      .arg(image));
                return;
            }

            if (tileset->tileCount() == 0) {
                QMessageBox::critical(this, tr("Error"),
                                      tr("No tiles found in the tileset image "
                                         "when using the given tile size, "
                                         "margin and spacing!"));
                return;
            }

            tileset->syncExpectedColumnsAndRows();
        }

        if (mMode == CreateTileset) {
            s->setValue(QLatin1String(COLOR_ENABLED_KEY), useTransparentColor);
            s->setValue(QLatin1String(COLOR_KEY), transparentColor.name());
            s->setValue(QLatin1String(TILE_SIZE_KEY), QSize(tileWidth, tileHeight));
            s->setValue(QLatin1String(SPACING_KEY), spacing);
            s->setValue(QLatin1String(MARGIN_KEY), margin);
        }
    } else {
        tileset = Tileset::create(name, 1, 1);
    }

    if (mMode == CreateTileset) {
        s->setValue(QLatin1String(TYPE_KEY), mUi->tilesetType->currentIndex());
        s->setValue(QLatin1String(EMBED_KEY), mUi->embedded->isChecked());
    }

    mNewTileset = tileset;
    accept();
}

void NewTilesetDialog::setMode(Mode mode)
{
    mMode = mode;

    if (mode == EditTilesetParameters) {
        mUi->tilesetType->setCurrentIndex(0);
        setWindowTitle(QApplication::translate("NewTilesetDialog", "Edit Tileset"));
    } else {
        setWindowTitle(QApplication::translate("NewTilesetDialog", "New Tileset"));
    }

    mUi->tilesetGroupBox->setVisible(mode == CreateTileset);
    updateOkButton();
}

void NewTilesetDialog::browse()
{
    const QString filter = Utils::readableImageFormatsFilter();
    QString f = QFileDialog::getOpenFileName(this, tr("Tileset Image"), mPath,
                                             filter);
    if (!f.isEmpty()) {
        mUi->image->setText(f);
        mPath = f;

        if (!mNameWasEdited)
            mUi->name->setText(QFileInfo(f).completeBaseName());
    }
}

void NewTilesetDialog::nameEdited(const QString &name)
{
    mNameWasEdited = !name.isEmpty();
}

void NewTilesetDialog::tilesetTypeChanged(int index)
{
    mUi->imageGroupBox->setVisible(index == 0);
    updateOkButton();
}

void NewTilesetDialog::updateOkButton()
{
    QPushButton *okButton = mUi->buttonBox->button(QDialogButtonBox::Ok);

    bool enabled = true;
    QString text;

    if (mMode == CreateTileset) {
        enabled &= !mUi->name->text().isEmpty();
        text = isEmbedded() ? tr("&OK") : tr("&Save As...");
    } else {
        text = tr("&OK");
    }

    if (tilesetType(mUi) == TilesetImage)
        enabled &= !mUi->image->text().isEmpty();

    okButton->setEnabled(enabled);
    okButton->setText(text);
}

void NewTilesetDialog::updateColorPickerButton()
{
    mUi->dropperButton->setEnabled(mUi->useTransparentColor->isChecked() &&
                                   !mUi->image->text().isEmpty());
}

/**
 * Shows the popup window used to select the color from the chosen image.
 */
void NewTilesetDialog::pickColorFromImage()
{
    auto *popup = new ImageColorPickerWidget(mUi->dropperButton);
    popup->setAttribute(Qt::WA_DeleteOnClose);

    connect(popup, &ImageColorPickerWidget::colorSelected,
            this, &NewTilesetDialog::colorSelected);

    if (!popup->selectColor(mUi->image->text()))
        delete popup;
}

void NewTilesetDialog::colorSelected(QColor color)
{
    mUi->colorButton->setColor(color);
}
