/*
 * issuesdock.h
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

#include <QDockWidget>

class QListView;
class QSortFilterProxyModel;

namespace Tiled {

class FilterEdit;
class IssueFilterModel;

struct Issue
{
    enum Severity {
        Error,
        Warning
    };

    Issue() = default;
    Issue(Severity severity, QString text)
        : severity(severity)
        , text(text)
    {}

    Severity severity = Error;
    QString text;

    int occurrences() const { return mOccurrences; }

    bool operator==(const Issue &o) const
    {
        return severity == o.severity
                && text == o.text;
    }

private:
    friend class IssuesModel;
    int mOccurrences = 1;
};

/**
 * A dock widget that shows errors and warnings, along with the ability to
 * filter them.
 */
class IssuesDock : public QDockWidget
{
    Q_OBJECT

public:
    IssuesDock(QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *e) override;

private:
    void retranslateUi();

    IssueFilterModel *mProxyModel;
    FilterEdit *mFilterEdit;
    QListView *mIssuesView;
};

void reportIssue(const Issue &issue);

inline void reportError(const QString &text)
{
    reportIssue(Issue { Issue::Error, text });
}

inline void reportWarning(const QString &text)
{
    reportIssue(Issue { Issue::Warning, text });
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Issue)
