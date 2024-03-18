/*
 * actionsearch.cpp
 * Copyright 2022, Chris Boehm AKA dogboydog
 * Copyright 2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "actionsearch.h"

#include "actionmanager.h"
#include "utils.h"

#include <QAction>
#include <QApplication>
#include <QPainter>
#include <QStaticText>
#include <QStyledItemDelegate>

namespace Tiled {

static QFont scaledFont(const QFont &font, qreal scale)
{
    QFont scaled(font);
    if (font.pixelSize() > 0)
        scaled.setPixelSize(font.pixelSize() * scale);
    else
        scaled.setPointSizeF(font.pointSizeF() * scale);
    return scaled;
}

class ActionMatchDelegate : public QStyledItemDelegate
{
public:
    ActionMatchDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    void setWords(const QStringList &words) { mWords = words; }

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

    QStringList mWords;
};

ActionMatchDelegate::ActionMatchDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QSize ActionMatchDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const
{
    const QFont bigFont = scaledFont(option.font, 1.2);
    const QFontMetrics bigFontMetrics(bigFont);

    const int margin = Utils::dpiScaled(2);
    return QSize(margin * 2, margin * 2 + bigFontMetrics.lineSpacing());
}

void ActionMatchDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    painter->save();

    const QString name = index.data().toString();

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    const auto ranges = Utils::matchingRanges(mWords, &name);
#else
    const auto ranges = Utils::matchingRanges(mWords, name);
#endif

    QString nameHtml;
    int nameIndex = 0;

    auto nameRange = [&] (int first, int last) -> QStringRef {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        return QStringView(name).mid(first, last - first + 1);
#else
        return name.midRef(first, last - first + 1);
#endif
    };

    for (const auto &range : ranges) {
        if (range.first > nameIndex)
            nameHtml.append(nameRange(nameIndex, range.first - 1));

        nameHtml.append(QStringLiteral("<b>"));
        nameHtml.append(nameRange(range.first, range.second));
        nameHtml.append(QStringLiteral("</b>"));

        nameIndex = range.second + 1;
    }

    nameHtml.append(nameRange(nameIndex, name.size() - 1));

    const Fonts fonts(option.font);

    const int margin = Utils::dpiScaled(2);
    const int iconSize = option.rect.height() - margin * 2;
    const auto nameRect = option.rect.adjusted(margin * 4 + iconSize, margin, -margin, 0);
    const auto shortcutRect = option.rect.adjusted(0, margin, -margin, -margin);

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

    QStaticText staticText(nameHtml);
    staticText.setTextOption(textOption);
    staticText.setTextFormat(Qt::RichText);

    painter->setFont(fonts.big);
    painter->drawStaticText(nameRect.topLeft(), staticText);

    const auto icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (!icon.isNull()) {
        const auto iconRect = QRect(option.rect.topLeft() + QPoint(margin, margin),
                                    QSize(iconSize, iconSize));

        icon.paint(painter, iconRect);
    }

    const auto shortcut = index.data(ActionLocatorSource::ShortcutRole).value<QKeySequence>();
    if (!shortcut.isEmpty()) {
        const QString shortcutText = shortcut.toString(QKeySequence::NativeText);
        const QFontMetrics smallFontMetrics(fonts.small);

        staticText.setTextFormat(Qt::PlainText);
        staticText.setText(shortcutText);
        staticText.prepare(painter->transform(), fonts.small);

        const int centeringMargin = (shortcutRect.height() - smallFontMetrics.height()) / 2;
        painter->setOpacity(0.75);
        painter->setFont(fonts.small);
        painter->drawStaticText(shortcutRect.right() - staticText.size().width() - centeringMargin,
                                shortcutRect.top() + centeringMargin,
                                staticText);
    }

    // draw the focus rect
    if (option.state & QStyle::State_HasFocus) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(option);
        o.rect = style->subElementRect(QStyle::SE_ItemViewItemFocusRect, &option);
        o.state |= QStyle::State_KeyboardFocusChange;
        o.state |= QStyle::State_Item;
        QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                ? QPalette::Normal : QPalette::Disabled;
        o.backgroundColor = option.palette.color(cg, (option.state & QStyle::State_Selected)
                                                ? QPalette::Highlight : QPalette::Window);
        style->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter);
    }

    painter->restore();
}

///////////////////////////////////////////////////////////////////////////////

ActionLocatorSource::ActionLocatorSource(QObject *parent)
    : LocatorSource(parent)
    , mDelegate(new ActionMatchDelegate(this))
{}

int ActionLocatorSource::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mMatches.size();
}

QVariant ActionLocatorSource::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        const Match &match = mMatches.at(index.row());
        return match.text;
    }
    case Qt::DecorationRole: {
        const Match &match = mMatches.at(index.row());
        if (auto action = ActionManager::findAction(match.actionId))
            return action->icon();
        break;
    }
    case ShortcutRole: {
        const Match &match = mMatches.at(index.row());
        if (auto action = ActionManager::findAction(match.actionId))
            return action->shortcut();
        break;
    }
    }
    return QVariant();
}

QAbstractItemDelegate *ActionLocatorSource::delegate() const
{
    return mDelegate;
}

QString ActionLocatorSource::placeholderText() const
{
    return QCoreApplication::translate("Tiled::LocatorWidget", "Search actions...");
}

QVector<ActionLocatorSource::Match> ActionLocatorSource::findActions(const QStringList &words)
{
    static const QRegularExpression re(QLatin1String("(?<=^|[^&])&"));
    const QList<Id> actions = ActionManager::actions();
    const Id searchActionsId("SearchActions");

    QVector<Match> result;

    for (const Id &actionId : actions) {
        if (actionId == searchActionsId)
            continue;

        const auto action = ActionManager::findEnabledAction(actionId);
        if (!action)
            continue;

        // remove single & characters
        QString sanitizedText = action->text();
        sanitizedText.replace(re, QString());

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        const int totalScore = Utils::matchingScore(words, sanitizedText);
#else
        const int totalScore = Utils::matchingScore(words, &sanitizedText);
#endif

        if (totalScore > 0) {
            result.append(Match {
                              totalScore,
                              actionId,
                              sanitizedText
                          });
        }
    }

    return result;
}

void ActionLocatorSource::setFilterWords(const QStringList &words)
{
    auto matches = findActions(words);

    std::stable_sort(matches.begin(), matches.end(), [] (const Match &a, const Match &b) {
        // Sort based on score first
        if (a.score != b.score)
            return a.score > b.score;

        // If score is the same, sort alphabetically
        return a.text.compare(b.text, Qt::CaseInsensitive) < 0;
    });

    mDelegate->setWords(words);

    beginResetModel();
    mMatches = std::move(matches);
    endResetModel();
}

void ActionLocatorSource::activate(const QModelIndex &index)
{
    const Id actionId = mMatches.at(index.row()).actionId;
    if (auto action = ActionManager::findEnabledAction(actionId))
        action->trigger();
}

} // namespace Tiled

#include "moc_actionsearch.cpp"
