/*
 * imagecolorpickerwidget.h
 * Copyright 2016, Ava Brumfield <alturos@gmail.com>
 * Copyright 2016, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#pragma once

#include <QColor>
#include <QPixmap>
#include <QFrame>

namespace Ui {
class ImageColorPickerWidget;
}

namespace Tiled {

/**
 * A popup widget for selecting a colour from an image.
 */
class ImageColorPickerWidget : public QFrame
{
    Q_OBJECT

public:
    ImageColorPickerWidget(QWidget *parent = nullptr);
    ~ImageColorPickerWidget() override;

    bool selectColor(const QString &image);

signals:
    void colorSelected(QColor);

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    void onMouseMove(QMouseEvent*);
    void onMouseRelease(QMouseEvent*);

    Ui::ImageColorPickerWidget *mUi;
    QColor mPreviewColor;
    QColor mSelectedColor;
    QImage mImage;
    QPixmap mPreviewIcon;
    double mScaleX, mScaleY;
};

} // namespace Tiled
