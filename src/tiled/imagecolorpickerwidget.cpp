/*
 * imagecolorpickerwidget.cpp
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

#include "imagecolorpickerwidget.h"
#include "ui_imagecolorpickerwidget.h"

#include "utils.h"

#include <QMouseEvent>

using namespace Tiled;

ImageColorPickerWidget::ImageColorPickerWidget(QWidget *parent)
    : QFrame(parent, Qt::Popup)
    , mUi(new Ui::ImageColorPickerWidget)
{
    mUi->setupUi(this);

    connect(mUi->imageArea, &ClickableLabel::mouseMoved, this, &ImageColorPickerWidget::onMouseMove);
    connect(mUi->imageArea, &ClickableLabel::mouseReleased, this, &ImageColorPickerWidget::onMouseRelease);

    mPreviewIcon = QPixmap(Utils::dpiScaled(QSize(96, 24)));
    mPreviewIcon.fill(Qt::transparent);
    mUi->preview->setPixmap(mPreviewIcon);
}

ImageColorPickerWidget::~ImageColorPickerWidget()
{
    delete mUi;
}

bool ImageColorPickerWidget::selectColor(const QString &image)
{
    QPixmap pix(image);
    if (pix.isNull())
        return false;

    mImage = pix.toImage();
    mScaleX = 1;
    mScaleY = 1;

    const QRect screenRect = Utils::screenRect(parentWidget());
    const int maxW = screenRect.width() * 2 / 3;
    const int maxH = screenRect.height() * 2 / 3;

    if (mImage.width() > maxW || mImage.height() > maxH) {
        pix = pix.scaled(maxW, maxH, Qt::KeepAspectRatio);
        mScaleX = qMin<double>(mImage.width(), pix.width()) / qMax<double>(mImage.width(), pix.width());
        mScaleY = qMin<double>(mImage.height(), pix.height()) / qMax<double>(mImage.height(), pix.height());
    }

    mScaledImageSize = pix.size();

    mUi->imageArea->setPixmap(pix);
    mUi->imageArea->adjustSize();

    // Center the widget on the screen of its parent
    QRect desiredGeometry(QPoint(), sizeHint());
    desiredGeometry.moveCenter(screenRect.center());
    setGeometry(desiredGeometry);

    show();

    return true;
}

void ImageColorPickerWidget::onMouseMove(QMouseEvent *event)
{
    if (!mImage.isNull()) {
        QPoint imgPos = event->pos();

        // Correct for centering of the image
        imgPos.rx() -= (mUi->imageArea->width() - mScaledImageSize.width()) / 2;
        imgPos.ry() -= (mUi->imageArea->height() - mScaledImageSize.height()) / 2;

        // Correct for scaling of the image
        imgPos.rx() /= mScaleX;
        imgPos.ry() /= mScaleY;

        // Contains check avoids Qt printing a warning when out of range
        mPreviewColor = mImage.rect().contains(imgPos) ? mImage.pixel(imgPos) : QColor();
        if (!mPreviewColor.isValid())
            mPreviewColor = mSelectedColor;

        mPreviewIcon.fill(mPreviewColor);
        mUi->preview->setPixmap(mPreviewIcon);
        mUi->colorName->setText(mPreviewColor.name());
    } else {
        mPreviewColor = mSelectedColor;
    }
}

void ImageColorPickerWidget::onMouseRelease(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        if (!mImage.isNull()) {
            mSelectedColor = mPreviewColor;
            emit colorSelected(mSelectedColor);
            close();
        }
    } else if (event->button() == Qt::RightButton) {
        close();
    }
}

#include "moc_imagecolorpickerwidget.cpp"
