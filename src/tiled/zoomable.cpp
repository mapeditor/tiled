/*
 * zoomable.cpp
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

#include "zoomable.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPinchGesture>
#include <QValidator>

#include <cmath>

#include "qtcompat_p.h"

using namespace Tiled;

static QString scaleToString(qreal scale)
{
    return QString(QLatin1String("%1 %")).arg(int(scale * 100));
}


Zoomable::Zoomable(QObject *parent)
    : QObject(parent)
    , mScale(1)
    , mGestureStartScale(0)
    , mComboBox(nullptr)
    , mComboRegExp(QLatin1String("^\\s*(\\d+)\\s*%?\\s*$"))
    , mComboValidator(nullptr)
{
    mZoomFactors = QVector<qreal> {
        0.015625,
        0.03125,
        0.0625,
        0.125,
        0.25,
        0.33,
        0.5,
        0.75,
        1.0,
        1.5,
        2.0,
        3.0,
        4.0,
        5.5,
        8.0,
        11.0,
        16.0,
        23.0,
        32.0,
        45.0,
        64.0,
        90.0,
        128.0,
        180.0,
        256.0
    };
}

void Zoomable::setScale(qreal scale)
{
    if (scale == mScale)
        return;

    mScale = scale;

    syncComboBox();

    emit scaleChanged(mScale);
}

bool Zoomable::canZoomIn() const
{
    return mScale < mZoomFactors.last();
}

bool Zoomable::canZoomOut() const
{
    return mScale > mZoomFactors.first();
}

void Zoomable::handleWheelDelta(int delta)
{
    if (delta <= -120) {
        zoomOut();
    } else if (delta >= 120) {
        zoomIn();
    } else {
        // We're dealing with a finer-resolution mouse. Allow it to have finer
        // control over the zoom level.
        qreal factor = 1 + 0.3 * qAbs(qreal(delta) / 8 / 15);
        if (delta < 0)
            factor = 1 / factor;

        qreal scale = qBound(mZoomFactors.first(),
                             mScale * factor,
                             mZoomFactors.last());

        // Round to at most four digits after the decimal point
        setScale(std::floor(scale * 10000 + 0.5) / 10000);
    }
}

void Zoomable::handlePinchGesture(QPinchGesture *pinch)
{
    if (!(pinch->changeFlags() & QPinchGesture::ScaleFactorChanged))
        return;

    switch (pinch->state()) {
    case Qt::NoGesture:
        break;
    case Qt::GestureStarted:
        mGestureStartScale = mScale;
        Q_FALLTHROUGH();
    case Qt::GestureUpdated: {
        qreal factor = pinch->totalScaleFactor();
        qreal scale = qBound(mZoomFactors.first(),
                             mGestureStartScale * factor,
                             mZoomFactors.last());
        setScale(std::floor(scale * 10000 + 0.5) / 10000);
        break;
    }
    case Qt::GestureFinished:
    case Qt::GestureCanceled:
        break;
    }
}

void Zoomable::zoomIn()
{
    for (qreal scale : qAsConst(mZoomFactors)) {
        if (scale > mScale) {
            setScale(scale);
            break;
        }
    }
}

void Zoomable::zoomOut()
{
    for (int i = mZoomFactors.count() - 1; i >= 0; --i) {
        if (mZoomFactors[i] < mScale) {
            setScale(mZoomFactors[i]);
            break;
        }
    }
}

void Zoomable::resetZoom()
{
    setScale(1);
}

void Zoomable::setZoomFactors(const QVector<qreal>& factors)
{
    mZoomFactors = factors;
}

void Zoomable::setComboBox(QComboBox *comboBox)
{
    if (mComboBox) {
        mComboBox->disconnect(this);
        if (mComboBox->lineEdit())
            mComboBox->lineEdit()->disconnect(this);
        mComboBox->setValidator(nullptr);
    }

    mComboBox = comboBox;

    if (mComboBox) {
        mComboBox->clear();
        for (qreal scale : qAsConst(mZoomFactors))
            mComboBox->addItem(scaleToString(scale), scale);
        syncComboBox();
        connect(mComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
                this, &Zoomable::comboActivated);

        mComboBox->setEditable(true);
        mComboBox->setInsertPolicy(QComboBox::NoInsert);
        connect(mComboBox->lineEdit(), &QLineEdit::editingFinished,
                this, &Zoomable::comboEdited);

        if (!mComboValidator)
            mComboValidator = new QRegExpValidator(mComboRegExp, this);
        mComboBox->setValidator(mComboValidator);
    }
}

void Zoomable::comboActivated(int index)
{
    setScale(mComboBox->itemData(index).toReal());
}

void Zoomable::comboEdited()
{
    int pos = mComboRegExp.indexIn(mComboBox->currentText());
    Q_ASSERT(pos != -1);
    Q_UNUSED(pos)

    qreal scale = qBound(mZoomFactors.first(),
                         qreal(mComboRegExp.cap(1).toDouble() / 100.f),
                         mZoomFactors.last());

    setScale(scale);
}

void Zoomable::syncComboBox()
{
    if (!mComboBox)
        return;

    int index = mComboBox->findData(mScale);
    // For a custom scale, the current index must be set to -1
    mComboBox->setCurrentIndex(index);
    mComboBox->setEditText(scaleToString(mScale));
}
