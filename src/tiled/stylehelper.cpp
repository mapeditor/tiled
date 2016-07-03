/*
 * stylehelper.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "stylehelper.h"

#include "preferences.h"

#include <QApplication>
#include <QStyle>
#include <QStyleFactory>

namespace Tiled {
namespace Internal {

QString StyleHelper::mDefaultStyle;
QPalette StyleHelper::mDefaultPalette;

static QPalette createPalette(const QColor &windowColor,
                              const QColor &highlightColor)
{
    int hue, sat, windowValue;
    windowColor.getHsv(&hue, &sat, &windowValue);

    auto fromValue = [=](int value) {
        return QColor::fromHsv(hue, sat, qMin(255, qMax(0, value)));
    };

    const bool isLight = windowValue > 128;
    const int baseValue = isLight ? windowValue + 48 : windowValue - 24;

    const int lightTextValue = qMin(255, baseValue + 192);
    const int darkTextValue = qMax(0, baseValue - 192);
    const int lightTextDisabledValue = (baseValue + lightTextValue) / 2;
    const int darkTextDisabledValue = (baseValue + darkTextValue) / 2;

    const QColor lightText = fromValue(lightTextValue);
    const QColor darkText = fromValue(darkTextValue);
    const QColor lightDisabledText = fromValue(lightTextDisabledValue);
    const QColor darkDisabledText = fromValue(darkTextDisabledValue);

    QPalette palette(fromValue(windowValue));
    palette.setColor(QPalette::Base, fromValue(baseValue));
    palette.setColor(QPalette::AlternateBase, fromValue(baseValue - 10));
    palette.setColor(QPalette::Window, fromValue(windowValue));
    palette.setColor(QPalette::WindowText, isLight ? darkText : lightText);
    palette.setColor(QPalette::ButtonText, isLight ? darkText : lightText);
    palette.setColor(QPalette::Text, isLight ? darkText : lightText);
    palette.setColor(QPalette::Light, QColor(255, 255, 255, 55));
    palette.setColor(QPalette::Dark, fromValue(windowValue - 55));

    palette.setColor(QPalette::Disabled, QPalette::WindowText, isLight ? darkDisabledText : lightDisabledText);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, isLight ? darkDisabledText : lightDisabledText);
    palette.setColor(QPalette::Disabled, QPalette::Text, isLight ? darkDisabledText : lightDisabledText);

    bool highlightIsDark = qGray(highlightColor.rgb()) < 120;
    palette.setColor(QPalette::Highlight, highlightColor);
    palette.setColor(QPalette::HighlightedText, highlightIsDark ? Qt::white : Qt::black);

    return palette;
}

void StyleHelper::initialize()
{
    mDefaultStyle = QApplication::style()->objectName();
    mDefaultPalette = QApplication::palette();

    Preferences *preferences = Preferences::instance();

    auto apply = [=]() {
        QString desiredStyle;
        QPalette desiredPalette;

        switch (preferences->applicationStyle()) {
        default:
        case Preferences::SystemDefaultStyle:
            desiredStyle = defaultStyle();
            desiredPalette = defaultPalette();
            break;
        case Preferences::FusionStyle:
            desiredStyle = QLatin1String("fusion");
            desiredPalette = createPalette(preferences->baseColor(),
                                           preferences->selectionColor());
            break;
        }

        if (QApplication::style()->objectName() != desiredStyle)
            QApplication::setStyle(QStyleFactory::create(desiredStyle));

        if (QApplication::palette() != desiredPalette)
            QApplication::setPalette(desiredPalette);
    };

    apply();

    QObject::connect(preferences, &Preferences::applicationStyleChanged, apply);
    QObject::connect(preferences, &Preferences::baseColorChanged, apply);
    QObject::connect(preferences, &Preferences::selectionColorChanged, apply);
}

} // namespace Internal
} // namespace Tiled
