/*
 * issuesmodel.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "issuesmodel.h"

namespace Tiled {

IssuesModel::IssuesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    mErrorIcon.addFile(QLatin1String("://images/16/dialog-error.png"));
    mErrorIcon.addFile(QLatin1String("://images/24/dialog-error.png"));
    mErrorIcon.addFile(QLatin1String("://images/32/dialog-error.png"));

    mWarningIcon.addFile(QLatin1String("://images/16/dialog-warning.png"));
    mWarningIcon.addFile(QLatin1String("://images/24/dialog-warning.png"));
    mWarningIcon.addFile(QLatin1String("://images/32/dialog-warning.png"));

    connect(&LoggingInterface::instance(), &LoggingInterface::issue,
            this, &IssuesModel::addIssue);
    connect(&LoggingInterface::instance(), &LoggingInterface::removeIssuesWithContext,
            this, &IssuesModel::removeIssuesWithContext);
}

IssuesModel &IssuesModel::instance()
{
    static IssuesModel issuesModel;
    return issuesModel;
}

void IssuesModel::addIssue(const Issue &issue)
{
    int i = mIssues.indexOf(issue);
    if (i != -1) {
        auto &existingIssue = mIssues[i];
        existingIssue.addOccurrence(issue);

        QModelIndex modelIndex = index(i);
        emit dataChanged(modelIndex, modelIndex);
        return;
    }

    switch (issue.severity()) {
    case Issue::Error: ++mErrorCount; break;
    case Issue::Warning: ++mWarningCount; break;
    }

    beginInsertRows(QModelIndex(), mIssues.size(), mIssues.size());
    mIssues.append(issue);
    endInsertRows();
}

void IssuesModel::removeIssues(const QList<unsigned> &issueIds)
{
    RangeSet<int> indexes;

    for (unsigned id : issueIds) {
        auto it = std::find_if(mIssues.cbegin(), mIssues.cend(),
                               [id] (const Issue &issue) { return issue.id() == id; });

        if (it != mIssues.cend())
            indexes.insert(std::distance(mIssues.cbegin(), it));
    }

    removeIssues(indexes);
}

void IssuesModel::removeIssuesWithContext(const void *context)
{
    RangeSet<int> indexes;

    for (int i = 0, size = mIssues.size(); i < size; ++i)
        if (mIssues.at(i).context() == context)
            indexes.insert(i);

    removeIssues(indexes);
}

void IssuesModel::removeIssues(const RangeSet<int> &indexes)
{
    if (indexes.isEmpty())
        return;

    // Remove back to front to keep the indexes valid
    RangeSet<int>::Range it = indexes.end();
    RangeSet<int>::Range begin = indexes.begin();
    // assert: end != begin, since there is at least one entry
    do {
        --it;

        std::for_each(mIssues.begin() + it.first(),
                      mIssues.begin() + it.last() + 1,
                      [this] (const Issue &issue) {
            switch (issue.severity()) {
            case Issue::Error: --mErrorCount; break;
            case Issue::Warning: --mWarningCount; break;
            }
        });

        beginRemoveRows(QModelIndex(), it.first(), it.last());
        mIssues.remove(it.first(), it.length());
        endRemoveRows();
    } while (it != begin);
}

void IssuesModel::clear()
{
    beginResetModel();

    mErrorCount = 0;
    mWarningCount = 0;
    mIssues.clear();

    endResetModel();
}

int IssuesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mIssues.size();
}

QVariant IssuesModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        return mIssues.at(index.row()).text();
    case Qt::DecorationRole:
        switch (mIssues.at(index.row()).severity()) {
        case Issue::Error:
            return mErrorIcon;
        case Issue::Warning:
            return mWarningIcon;
        }
        break;
    case Qt::BackgroundRole: {
        switch (mIssues.at(index.row()).severity()) {
        case Issue::Error:
            return QColor(253, 0, 69, 32);
        case Issue::Warning:
            return QColor(255, 230, 0, 32);
        }
        break;
    }
    case IssueRole:
        return QVariant::fromValue(mIssues.at(index.row()));
    }

    return QVariant();
}

} // namespace Tiled
