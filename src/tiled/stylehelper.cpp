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
#include "tiledproxystyle.h"

#include <QApplication>
#include <QPixmapCache>
#include <QStyle>
#include <QStyleFactory>

namespace Tiled {

StyleHelper *StyleHelper::mInstance;

static QPalette createPalette(const QColor &windowColor,
                              const QColor &highlightColor)
{
    int hue, sat, windowValue;
    windowColor.getHsv(&hue, &sat, &windowValue);

    auto fromValue = [=](int value) {
        return QColor::fromHsv(hue, sat, qBound(0, value, 255));
    };

    auto fromValueHalfSat = [=](int value) {
        return QColor::fromHsv(hue, sat / 2, qBound(0, value, 255));
    };

    const bool isLight = windowValue > 128;
    const int baseValue = isLight ? windowValue + 48 : windowValue - 24;

    const int lightTextValue = qMin(255, windowValue + 160);
    const int darkTextValue = qMax(0, windowValue - 160);

    const QColor lightText = QColor(lightTextValue, lightTextValue, lightTextValue);
    const QColor darkText = QColor(darkTextValue, darkTextValue, darkTextValue);
    const QColor lightDisabledText = QColor(lightTextValue, lightTextValue, lightTextValue, 128);
    const QColor darkDisabledText = QColor(darkTextValue, darkTextValue, darkTextValue, 128);

    QPalette palette(fromValue(windowValue));
    palette.setColor(QPalette::Base, fromValueHalfSat(baseValue));
    palette.setColor(QPalette::AlternateBase, fromValueHalfSat(baseValue - 10));
    palette.setColor(QPalette::WindowText, isLight ? darkText : lightText);
    palette.setColor(QPalette::ButtonText, isLight ? darkText : lightText);
    palette.setColor(QPalette::Text, isLight ? darkText : lightText);
    palette.setColor(QPalette::Light, fromValueHalfSat(windowValue + 55));
    palette.setColor(QPalette::Dark, fromValueHalfSat(windowValue - 55));
    palette.setColor(QPalette::Mid, fromValue(windowValue - 27));
    palette.setColor(QPalette::Midlight, fromValue(windowValue + 27));

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
    Q_ASSERT(!mInstance);
    mInstance = new StyleHelper;
}

StyleHelper::StyleHelper()
    : mDefaultStyle(QApplication::style()->objectName())
    , mDefaultPalette(QApplication::palette())
{
    apply();

    Preferences *preferences = Preferences::instance();
    QObject::connect(preferences, &Preferences::applicationStyleChanged, this, &StyleHelper::apply);
    QObject::connect(preferences, &Preferences::baseColorChanged, this, &StyleHelper::apply);
    QObject::connect(preferences, &Preferences::selectionColorChanged, this, &StyleHelper::apply);
}

void StyleHelper::apply()
{
    Preferences *preferences = Preferences::instance();

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
    case Preferences::TiledStyle:
        desiredStyle = QLatin1String("tiled");
        desiredPalette = createPalette(preferences->baseColor(),
                                       preferences->selectionColor());
        break;
    }

    if (QApplication::style()->objectName() != desiredStyle) {
        QStyle *style;

        if (desiredStyle == QLatin1String("tiled")) {
            style = QStyleFactory::create(QLatin1String("fusion"));
            style = new TiledProxyStyle(desiredPalette, style);
        } else {
            style = QStyleFactory::create(desiredStyle);
        }

        QApplication::setStyle(style);
    }

    if (QApplication::palette() != desiredPalette) {
        QPixmapCache::clear();
        QApplication::setPalette(desiredPalette);

        if (auto *style = qobject_cast<TiledProxyStyle*>(QApplication::style()))
            style->setPalette(desiredPalette);
    }

    emit styleApplied();
}

} // namespace Tiled
