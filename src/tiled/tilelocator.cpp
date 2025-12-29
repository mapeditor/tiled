/*
 * tilelocator.cpp
 * Copyright 2025, Siraj Ahmadzai
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

#include "tilelocator.h"

#include "documentmanager.h"
#include "mapdocument.h"
#include "mapview.h"
#include "maprenderer.h"
#include "layer.h"
#include "utils.h"
#include "locatorwidget.h"

#include <QApplication>
#include <QPainter>
#include <QRegularExpression>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QStaticText>
#include <QTextOption>

namespace Tiled {

///////////////////////////////////////////////////////////////////////////////

class TileMatchDelegate : public QStyledItemDelegate
{
public:
    explicit TileMatchDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        painter->save();

        const QString displayText = index.data().toString();
        const QString description = index.data(Qt::UserRole).toString();

        const Fonts fonts(option.font);
        const QFontMetrics bigFontMetrics(fonts.big);

        const int margin = Utils::dpiScaled(2);
        const auto displayRect = option.rect.adjusted(margin, margin, -margin, 0);
        const auto descriptionRect = option.rect.adjusted(margin, margin + bigFontMetrics.lineSpacing(), -margin, 0);

        // draw the background (covers selection)
        QStyle *style = QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

        // adjust text color to state
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                              ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
            cg = QPalette::Inactive;
        if (option.state & QStyle::State_Selected) {
            painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
        } else {
            painter->setPen(option.palette.color(cg, QPalette::Text));
        }

        QTextOption textOption;
        textOption.setWrapMode(QTextOption::NoWrap);

        QStaticText staticText(displayText);
        staticText.setTextOption(textOption);

        painter->setFont(fonts.big);
        painter->drawStaticText(displayRect.topLeft(), staticText);

        if (!description.isEmpty()) {
            staticText.setText(description);
            painter->setOpacity(0.75);
            painter->setFont(fonts.small);
            painter->drawStaticText(descriptionRect.topLeft(), staticText);
        }

        // draw the focus rect
        if (option.state & QStyle::State_HasFocus) {
            QStyleOptionFocusRect focusOption;
            focusOption.initFrom(option.widget);
            focusOption.rect = option.rect;
            focusOption.state = option.state;
            style->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOption, painter);
        }

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
    {
        const Fonts fonts(option.font);
        const QFontMetrics bigFontMetrics(fonts.big);
        const QFontMetrics smallFontMetrics(fonts.small);

        const QString displayText = index.data().toString();
        const QString description = index.data(Qt::UserRole).toString();

        const int margin = Utils::dpiScaled(2);
        const int height = margin * 2 + bigFontMetrics.lineSpacing();

        if (!description.isEmpty())
            return QSize(option.rect.width(), height + smallFontMetrics.lineSpacing());

        return QSize(option.rect.width(), height);
    }

private:
    class Fonts {
    public:
        Fonts(const QFont &base)
            : small(scaledFont(base, 0.9))
            , big(scaledFont(base, 1.1))
        {}

        const QFont small;
        const QFont big;
    };

    static QFont scaledFont(const QFont &font, qreal scale)
    {
        QFont scaled(font);
        if (font.pixelSize() > 0)
            scaled.setPixelSize(font.pixelSize() * scale);
        else
            scaled.setPointSizeF(font.pointSizeF() * scale);
        return scaled;
    }
};

///////////////////////////////////////////////////////////////////////////////

TileLocatorSource::TileLocatorSource(QObject *parent)
    : LocatorSource(parent)
    , mDelegate(new TileMatchDelegate(this))
{}

int TileLocatorSource::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mMatches.size();
}

QVariant TileLocatorSource::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        const Match &match = mMatches.at(index.row());
        return match.displayText;
    }
    case Qt::UserRole: {
        const Match &match = mMatches.at(index.row());
        return match.description;
    }
    }
    return QVariant();
}

QAbstractItemDelegate *TileLocatorSource::delegate() const
{
    return mDelegate;
}

QString TileLocatorSource::placeholderText() const
{
    return QCoreApplication::translate("Tiled::LocatorWidget", "Enter tile coordinates (e.g., 10,20 or x:10 y:20)");
}

void TileLocatorSource::activate(const QModelIndex &index)
{
    const Match &match = mMatches.at(index.row());
    
    auto documentManager = DocumentManager::instance();
    auto mapDocument = qobject_cast<MapDocument*>(documentManager->currentDocument());
    
    if (!mapDocument) {
        // Try to find any open map document
        const auto documents = documentManager->documents();
        for (auto document : documents) {
            if (auto mapDoc = qobject_cast<MapDocument*>(document.data())) {
                mapDocument = mapDoc;
                break;
            }
        }
    }
    
    if (!mapDocument)
        return;

    auto mapView = documentManager->viewForDocument(mapDocument);
    
    if (!mapView)
        return;

    // Convert tile coordinates to screen coordinates
    auto renderer = mapDocument->renderer();
    auto screenPos = renderer->tileToScreenCoords(match.tilePos);
    
    // Center the view on the specified tile position
    mapView->forceCenterOn(screenPos);
}

void TileLocatorSource::setFilterWords(const QStringList &words)
{
    auto matches = parseCoordinates(words);

    beginResetModel();
    mMatches = std::move(matches);
    endResetModel();
}

QVector<TileLocatorSource::Match> TileLocatorSource::parseCoordinates(const QStringList &words)
{
    QVector<Match> result;
    
    if (words.isEmpty())
        return result;

    // Join all words to handle cases like "10,20" or "x:10 y:20"
    QString input = words.join(QStringLiteral(" "));
    
    QPoint tilePos;
    if (parseCoordinate(input, tilePos)) {
        Match match;
        match.tilePos = tilePos;
        match.displayText = QCoreApplication::translate("Tiled::TileLocatorSource", "Jump to tile (%1, %2)")
                           .arg(tilePos.x()).arg(tilePos.y());
        match.description = QCoreApplication::translate("Tiled::TileLocatorSource", "Press Enter to jump to this tile");
        result.append(match);
    }
    
    return result;
}

bool TileLocatorSource::parseCoordinate(const QString &text, QPoint &tilePos)
{
    // Try different coordinate formats
    
    // Format 1: "x,y" (e.g., "10,20")
    QRegularExpression commaPattern(QStringLiteral(R"((\d+)\s*,\s*(\d+))"));
    auto match = commaPattern.match(text);
    if (match.hasMatch()) {
        tilePos.setX(match.captured(1).toInt());
        tilePos.setY(match.captured(2).toInt());
        return true;
    }
    
    // Format 2: "x:10 y:20" or "x:10, y:20"
    QRegularExpression xyPattern(QStringLiteral(R"(x\s*:\s*(\d+)(?:\s*[,\s]\s*y\s*:\s*(\d+))?)"));
    match = xyPattern.match(text);
    if (match.hasMatch()) {
        tilePos.setX(match.captured(1).toInt());
        if (match.captured(2).isEmpty()) {
            // Only x coordinate provided, try to find y in the rest of the text
            QString remaining = text.mid(match.capturedEnd());
            QRegularExpression yPattern(QStringLiteral(R"(y\s*:\s*(\d+))"));
            auto yMatch = yPattern.match(remaining);
            if (yMatch.hasMatch()) {
                tilePos.setY(yMatch.captured(1).toInt());
                return true;
            }
        } else {
            tilePos.setY(match.captured(2).toInt());
            return true;
        }
    }
    
    // Format 3: Just two numbers separated by space (e.g., "10 20")
    QRegularExpression spacePattern(QStringLiteral(R"((\d+)\s+(\d+))"));
    match = spacePattern.match(text);
    if (match.hasMatch()) {
        tilePos.setX(match.captured(1).toInt());
        tilePos.setY(match.captured(2).toInt());
        return true;
    }
    
    // Format 4: Single number (assume it's x coordinate)
    QRegularExpression singlePattern(QStringLiteral(R"((\d+))"));
    match = singlePattern.match(text);
    if (match.hasMatch()) {
        tilePos.setX(match.captured(1).toInt());
        tilePos.setY(0); // Default y to 0
        return true;
    }
    
    return false;
}

} // namespace Tiled

#include "moc_tilelocator.cpp" 