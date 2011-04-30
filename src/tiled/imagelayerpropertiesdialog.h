/*
 * imagelayerpropertiesdialog.h
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

#ifndef IMAGELAYERPROPERTIESDIALOG_H
#define IMAGELAYERPROPERTIESDIALOG_H

#include "propertiesdialog.h"
#include <QValidator>

class QLineEdit;
class QPushButton;

namespace Tiled {

class ImageLayer;

namespace Internal {

class MapDocument;
class ColorButton;

class PathValidator : public QValidator
{
    Q_OBJECT

public:
    explicit PathValidator(QObject *parent = 0);
    QValidator::State validate(QString &, int &) const;

private:
    Q_DISABLE_COPY(PathValidator)
};

class ImageLayerPropertiesDialog : public PropertiesDialog
{
    Q_OBJECT

public:
    ImageLayerPropertiesDialog(MapDocument *mapDocument,
                               ImageLayer *imageLayer,
                               QWidget *parent = 0);

    void accept();

private slots:
    void browseForImage();
    void imagePathChanged();

private:
    MapDocument *mMapDocument;
    ImageLayer *mImageLayer;
    ColorButton *mColorButton;
    QLineEdit *mImage;
    QPushButton *mBrowseButton;
};

} // namespace Internal
} // namespace Tiled

#endif // IMAGELAYERPROPERTIESDIALOG_H
