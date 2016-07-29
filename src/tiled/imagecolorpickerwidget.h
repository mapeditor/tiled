/*
 * imagecolorpickerwidget.h
 * Copyright 2009-2016, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef IMAGECOLORPICKERWIDGET_H
#define IMAGECOLORPICKERWIDGET_H

#include <QColor>
#include <QPixmap>
#include <QMouseEvent>
#include <QDialog>

namespace Ui {
class imageColorPickerWidget;
}

namespace Tiled {
namespace Internal {

/**
 * A popup widget for selecting a colour from an image.
 */
class ImageColorPickerWidget : public QWidget
{
    Q_OBJECT

public:
    ImageColorPickerWidget(QWidget *parent = 0);
    ~ImageColorPickerWidget();

    bool selectColor(const QString &image);
    QColor mSelectedColor;

signals:
    void colorSelected(QColor);

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    const QString mTitle = tr("Tileset Image");
    Ui::imageColorPickerWidget *mUi;
    QColor mPreviewColor;
    QImage mImage;
    QPixmap mPreviewIcon;
    double mScaleX, mScaleY;

private slots:
    void onMouseMove(QMouseEvent*);
    void onMousePress(QMouseEvent*);
    void onMouseRelease(QMouseEvent*);
    QRect findScreen() const;
};

} // namespace Internal
} // namespace Tiled

#endif // IMAGECOLORPICKERWIDGET_H
