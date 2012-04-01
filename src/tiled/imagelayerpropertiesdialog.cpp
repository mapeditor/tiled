/*
 * imagelayerpropertiesdialog.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Michael Woerister <michaelwoerister@gmail.com>
 * Copyright 2011, Gregory Nickonov <gregory@nickonov.ru>
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

#include "imagelayerpropertiesdialog.h"

#include "mapdocument.h"
#include "imagelayer.h"
#include "colorbutton.h"
#include "changeimagelayerproperties.h"
#include "mainwindow.h"
#include "utils.h"

#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QUndoStack>
#include <QGridLayout>
#include <QCoreApplication>
#include <QValidator>

using namespace Tiled;
using namespace Tiled::Internal;

PathValidator::PathValidator(QObject *parent)
    : QValidator(parent)
{
}

QValidator::State PathValidator::validate(QString &input, int &) const
{
    // TODO: Provide more intellectual file path checks here
    return QFile::exists(input) ? QValidator::Acceptable : QValidator::Invalid;
}

ImageLayerPropertiesDialog::ImageLayerPropertiesDialog(
    MapDocument *mapDocument,
    ImageLayer *imageLayer,
    QWidget *parent)
    : PropertiesDialog(tr("Image Layer"),
          imageLayer,
          mapDocument->undoStack(),
          parent)
    , mMapDocument(mapDocument)
    , mImageLayer(imageLayer)
    , mColorButton(new ColorButton)
{
    mBrowseButton = new QPushButton(tr("Browse..."));
    mImage = new QLineEdit(imageLayer->imageSource());
    mImage->setValidator(new PathValidator);

    QGridLayout *grid = new QGridLayout;

    grid->addWidget(new QLabel(tr("Image:")), 0, 0);
    grid->addWidget(mImage, 0, 1);
    grid->addWidget(mBrowseButton, 0, 2/*, 1, 1, Qt::AlignRight*/);
    grid->addWidget(new QLabel(tr("Color:")), 1, 0);
    grid->addWidget(mColorButton, 1, 1);

    connect(mBrowseButton, SIGNAL(clicked()), SLOT(browseForImage()));
    connect(mImage, SIGNAL(textEdited(QString)), SLOT(imagePathChanged()));

    mColorButton->setColor(mImageLayer->transparentColor().isValid()
        ? mImageLayer->transparentColor()
        : Qt::gray);

    qobject_cast<QBoxLayout*>(layout())->insertLayout(0, grid);
}

void ImageLayerPropertiesDialog::browseForImage()
{
    QString path = mImage->text();
    const QString filter = Utils::readableImageFormatsFilter();
    QString f = QFileDialog::getOpenFileName(this, tr("Layer Image"), path, filter);

    if (!f.isEmpty())
        mImage->setText(f);
}

void ImageLayerPropertiesDialog::imagePathChanged()
{
    QString newPath = mImage->text();

    if (!QFile::exists(newPath))
        mImage->setText(mImageLayer->imageSource());
}

void ImageLayerPropertiesDialog::accept()
{
    QUndoStack *undoStack = mMapDocument->undoStack();

    const QColor newColor = mColorButton->color() != Qt::gray
        ? mColorButton->color()
        : QColor();

    const QString newPath = mImage->text();
    const bool localChanges = newColor != mImageLayer->transparentColor() ||
            newPath != mImageLayer->imageSource();

    if (localChanges) {
        undoStack->beginMacro(QCoreApplication::translate(
            "Undo Commands",
            "Change Image Layer Properties"));

        undoStack->push(new ChangeImageLayerProperties(
                            mMapDocument,
                            mImageLayer,
                            newColor,
                            newPath));
    }

    PropertiesDialog::accept();

    if (localChanges)
        undoStack->endMacro();
}
