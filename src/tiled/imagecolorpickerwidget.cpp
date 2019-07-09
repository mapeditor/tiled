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

#include <QDesktopWidget>
#include <QMouseEvent>

using namespace Tiled;

ImageColorPickerWidget::ImageColorPickerWidget(QWidget *parent) :
    QFrame(parent, Qt::Popup),
    mUi(new Ui::ImageColorPickerWidget)
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

    QRectF rct = QApplication::desktop()->availableGeometry(this);
    double maxW = rct.width() * (2.0/3.0), maxH = rct.height() * (2.0/3.0);

    if (mImage.width() > maxW || mImage.height() > maxH) {
        pix = pix.scaled((int)maxW, (int)maxH, Qt::KeepAspectRatio);
        mScaleX = (double)qMin(mImage.width(), pix.width()) / (double)qMax(mImage.width(), pix.width());
        mScaleY = (double)qMin(mImage.height(), pix.height()) / (double)qMax(mImage.height(), pix.height());
    }

    mUi->imageArea->setPixmap(pix);
    mUi->imageArea->adjustSize();

    show();

    return true;
}

void ImageColorPickerWidget::onMouseMove(QMouseEvent* event)
{
    if (!mImage.isNull()) {
        mPreviewColor = mImage.pixel(event->pos().x() / mScaleX,
                                     event->pos().y() / mScaleY);
        if (!mPreviewColor.isValid())
            mPreviewColor = mSelectedColor;

        mPreviewIcon.fill(mPreviewColor);
        mUi->preview->setPixmap(mPreviewIcon);
        mUi->colorName->setText(mPreviewColor.name());
    } else {
        mPreviewColor = mSelectedColor;
    }
}

void ImageColorPickerWidget::onMouseRelease(QMouseEvent * event)
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

void ImageColorPickerWidget::resizeEvent(QResizeEvent *)
{
    move(QApplication::desktop()->availableGeometry(parentWidget()).center() - rect().center());
}
