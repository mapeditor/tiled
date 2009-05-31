/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "newtilesetdialog.h"
#include "ui_newtilesetdialog.h"

#include "tileset.h"

#include <QFileDialog>
#include <QImageReader>

using namespace Tiled;
using namespace Tiled::Internal;

NewTilesetDialog::NewTilesetDialog(const QString &path, QWidget *parent) :
    QDialog(parent),
    mPath(path),
    mUi(new Ui::NewTilesetDialog),
    mNameWasEdited(false)
{
    mUi->setupUi(this);

    connect(mUi->browseButton, SIGNAL(clicked()), SLOT(browse()));
    connect(mUi->name, SIGNAL(textEdited(QString)), SLOT(nameEdited(QString)));
}

NewTilesetDialog::~NewTilesetDialog()
{
    delete mUi;
}

Tileset *NewTilesetDialog::createTileset()
{
    if (exec() != QDialog::Accepted)
        return 0;

    const QString name = mUi->name->text();
    const QString image = mUi->image->text();
    const int tileWidth = mUi->tileWidth->value();
    const int tileHeight = mUi->tileHeight->value();
    const int spacing = mUi->spacing->value();
    // TODO: Add support for margin
    //const int margin = mUi->margin->value();

    Tileset *tileset = new Tileset(name,
                                   tileWidth,
                                   tileHeight,
                                   spacing);
    tileset->loadFromImage(image);
    return tileset;
}

void NewTilesetDialog::browse()
{
    // Build a filter with all supported image formats
    QString filter(tr("Image files"));
    filter += QLatin1String(" (");
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    bool first = true;
    foreach (const QByteArray &format, formats) {
        if (!first)
            filter += QLatin1Char(' ');
        first = false;
        filter += QLatin1String("*.");
        filter += QString::fromLatin1(format.toLower());
    }
    filter += QLatin1Char(')');

    QString f = QFileDialog::getOpenFileName(this, tr("Tileset Image"), mPath,
                                             filter);
    if (!f.isEmpty()) {
        mUi->image->setText(f);
        mPath = f;

        if (!mNameWasEdited)
            mUi->name->setText(QFileInfo(f).baseName());
    }
}

void NewTilesetDialog::nameEdited(const QString &name)
{
    mNameWasEdited = !name.isEmpty();
}
