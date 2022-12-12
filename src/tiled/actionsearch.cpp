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
#include <QCoreApplication>

namespace Tiled {

ActionLocatorSource::ActionLocatorSource(QObject *parent)
    : LocatorSource(parent)
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
    }
    }
    return QVariant();
}

QString ActionLocatorSource::placeholderText() const
{
    return QCoreApplication::translate("Tiled::LocatorWidget", "Search actions...");
}

QVector<ActionLocatorSource::Match> ActionLocatorSource::findActions(const QStringList &words)
{
    const QRegularExpression re(QLatin1String("(?<=^|[^&])&"));
    const QList<Id> actions = ActionManager::actions();

    QVector<Match> result;

    for (const Id &actionId : actions) {
        const QAction *action = ActionManager::action(actionId);
        if (!action->isEnabled())
            continue;

        // remove single & characters
        QString sanitizedText = action->text();
        sanitizedText.replace(re, QString());

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        const int totalScore = Utils::matchingScore(words, sanitizedText);
#else
        const int totalScore = Utils::matchingScore(words, QStringRef(&sanitizedText));
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

    beginResetModel();
    mMatches = std::move(matches);
    endResetModel();
}

void ActionLocatorSource::activate(const QModelIndex &index)
{
    const Id actionId = mMatches.at(index.row()).actionId;
    if (auto action = ActionManager::findAction(actionId))
        action->trigger();
}

} // namespace Tiled

#include "moc_actionsearch.cpp"
