/*
 * issuesmodel.h
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

#pragma once

#include "logginginterface.h"
#include "rangeset.h"

#include <QAbstractListModel>
#include <QIcon>
#include <QVector>

namespace Tiled {

class IssuesModel : public QAbstractListModel
{
    Q_OBJECT

    IssuesModel(QObject *parent = nullptr);

public:
    enum {
        IssueRole = Qt::UserRole
    };

    static IssuesModel &instance();

    void addIssue(const Issue &issue);
    void removeIssues(const QList<unsigned> &issueIds);
    void removeIssuesWithContext(const void *context);
    void clear();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    int errorCount() const;
    int warningCount() const;

    const QIcon &errorIcon() const;
    const QIcon &warningIcon() const;

signals:
    void counterClicked();

private:
    void removeIssues(const RangeSet<int> &indexes);

    QVector<Issue> mIssues;

    int mErrorCount = 0;
    int mWarningCount = 0;

    QIcon mErrorIcon;
    QIcon mWarningIcon;
};


inline int IssuesModel::errorCount() const
{
    return mErrorCount;
}

inline int IssuesModel::warningCount() const
{
    return mWarningCount;
}

inline const QIcon &IssuesModel::errorIcon() const
{
    return mErrorIcon;
}

inline const QIcon &IssuesModel::warningIcon() const
{
    return mWarningIcon;
}

} // namespace Tiled
