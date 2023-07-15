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
#include <QStyleHints>

#include <QtBoolPropertyManager>

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
    const int textValue = isLight ? qMax(0, windowValue - 160)
                                  : qMin(255, windowValue + 160);

    const QColor text { textValue, textValue, textValue };
    const QColor disabledText { textValue, textValue, textValue, 128 };

    QPalette palette(fromValue(windowValue));
    palette.setColor(QPalette::Base, fromValueHalfSat(baseValue));
    palette.setColor(QPalette::AlternateBase, fromValueHalfSat(baseValue - 10));
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Light, fromValueHalfSat(windowValue + 55));
    palette.setColor(QPalette::Dark, fromValueHalfSat(windowValue - 55));
    palette.setColor(QPalette::Mid, fromValue(windowValue - 27));
    palette.setColor(QPalette::Midlight, fromValue(windowValue + 27));

    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledText);

    bool highlightIsDark = qGray(highlightColor.rgb()) < 120;
    palette.setColor(QPalette::Highlight, highlightColor);
    palette.setColor(QPalette::HighlightedText, highlightIsDark ? Qt::white : Qt::black);
    palette.setColor(QPalette::PlaceholderText, disabledText);

    if (!isLight) {
        const QColor lightskyblue { 0x87, 0xce, 0xfa };
        palette.setColor(QPalette::Link, lightskyblue);
        palette.setColor(QPalette::LinkVisited, lightskyblue);
    }

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
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    , mDefaultShowShortcutsInContextMenus(QGuiApplication::styleHints()->showShortcutsInContextMenus())
#endif
{
    apply();
    applyFont();

    Preferences *preferences = Preferences::instance();
    QObject::connect(preferences, &Preferences::applicationStyleChanged, this, &StyleHelper::apply);
    QObject::connect(preferences, &Preferences::baseColorChanged, this, &StyleHelper::apply);
    QObject::connect(preferences, &Preferences::selectionColorChanged, this, &StyleHelper::apply);
    QObject::connect(preferences, &Preferences::applicationFontChanged, this, &StyleHelper::applyFont);
}

void StyleHelper::apply()
{
    Preferences *preferences = Preferences::instance();

    QString desiredStyle;
    QPalette desiredPalette;
    bool showShortcutsInContextMenus = true;

    switch (preferences->applicationStyle()) {
    default:
    case Preferences::SystemDefaultStyle:
        desiredStyle = defaultStyle();
        desiredPalette = defaultPalette();
        showShortcutsInContextMenus = mDefaultShowShortcutsInContextMenus;
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

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    QGuiApplication::styleHints()->setShowShortcutsInContextMenus(showShortcutsInContextMenus);
#else
    Q_UNUSED(showShortcutsInContextMenus)
#endif

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

    QtBoolPropertyManager::resetIcons();

    emit styleApplied();
}

void StyleHelper::applyFont()
{
    Preferences *prefs = Preferences::instance();

    if (prefs->useCustomFont()) {
        if (!mDefaultFont.has_value())
            mDefaultFont = QApplication::font();

        QApplication::setFont(prefs->customFont());

    } else if (mDefaultFont.has_value()) {
        QApplication::setFont(*mDefaultFont);
    }
}

} // namespace Tiled

#include "moc_stylehelper.cpp"
