/*
 * issuesdock.cpp
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

#include "issuesdock.h"

#include "filteredit.h"
#include "utils.h"

#include <QAbstractListModel>
#include <QCheckBox>
#include <QEvent>
#include <QGuiApplication>
#include <QIcon>
#include <QListView>
#include <QPainter>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include <QDebug>

#include <memory>

namespace Tiled {

class IssuesModel : public QAbstractListModel
{
public:
    enum {
        IssueRole = Qt::UserRole
    };

    static IssuesModel &instance();

    void addIssue(const Issue &issue);
    void clear();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    IssuesModel(QObject *parent = nullptr);

    QVector<Issue> mIssues;

    QIcon mErrorIcon;
    QIcon mWarningIcon;
};

IssuesModel::IssuesModel(QObject *parent)
    : QAbstractListModel(parent)
    , mErrorIcon(QIcon::fromTheme(QLatin1String("dialog-error")))
    , mWarningIcon(QIcon::fromTheme(QLatin1String("dialog-warning")))
{
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
        ++mIssues[i].occurrences;
        QModelIndex modelIndex = index(i);
        emit dataChanged(modelIndex, modelIndex);
        return;
    }

    beginInsertRows(QModelIndex(), mIssues.size(), mIssues.size());
    mIssues.append(issue);
    endInsertRows();
}

void IssuesModel::clear()
{
    beginResetModel();
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
        return mIssues.at(index.row()).text;
    case Qt::DecorationRole:
        switch (mIssues.at(index.row()).severity) {
        case Issue::Error:
            return mErrorIcon;
        case Issue::Warning:
            return mWarningIcon;
        }
        break;
    case Qt::BackgroundRole: {
        switch (mIssues.at(index.row()).severity) {
        case Issue::Error:
            return QColor(253, 0, 69, 32);
        case Issue::Warning:
            return QColor(255, 230, 0, 32);
        }
        break;
    }
    case Qt::ForegroundRole: {
        switch (mIssues.at(index.row()).severity) {
        case Issue::Error:
            return QColor(164, 0, 15);
        case Issue::Warning:
            return QColor(113, 81, 0);
        }
        break;
    }
    case IssueRole:
        return QVariant::fromValue(mIssues.at(index.row()));
    }

    return QVariant();
}


class IssueFilterModel : public QSortFilterProxyModel
{
public:
    IssueFilterModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {}

    void setShowWarnings(bool showWarnings)
    {
        if (mShowWarnings == showWarnings)
            return;

        mShowWarnings = showWarnings;
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (!mShowWarnings) {
            auto model = sourceModel();
            auto index = model->index(sourceRow, 0, sourceParent);
            auto issue = model->data(index, IssuesModel::IssueRole).value<Issue>();
            if (issue.severity == Issue::Warning)
                return false;
        }

        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }

private:
    bool mShowWarnings = true;
};


class IssueDelegate : public QStyledItemDelegate
{
public:
    IssueDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

void IssueDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    auto color = index.data(Qt::ForegroundRole).value<QBrush>().color();
    color.setAlpha(32);
    painter->setPen(color);
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
}


IssuesDock::IssuesDock(QWidget *parent)
    : QDockWidget(parent)
    , mProxyModel(new IssueFilterModel(this))
    , mFilterEdit(new FilterEdit)
    , mIssuesView(new QListView)
{
    setObjectName(QLatin1String("IssuesDock"));

    mProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setSourceModel(&IssuesModel::instance());

    mIssuesView->setModel(mProxyModel);
    mIssuesView->setIconSize(Utils::dpiScaled(QSize(16, 16)));
    mIssuesView->setItemDelegate(new IssueDelegate);
    mIssuesView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    mFilterEdit->setFilteredView(mIssuesView);

    connect(mFilterEdit, &QLineEdit::textChanged,
            mProxyModel, &QSortFilterProxyModel::setFilterFixedString);

    auto showWarningsCheckBox = new QCheckBox(tr("Show warnings"));
    showWarningsCheckBox->setChecked(true);

    auto clearButton = new QPushButton(tr("Clear"));

    connect(showWarningsCheckBox, &QCheckBox::toggled, mProxyModel, &IssueFilterModel::setShowWarnings);
    connect(clearButton, &QPushButton::clicked, &IssuesModel::instance(), &IssuesModel::clear);

    auto toolBarLayout = new QHBoxLayout;
    toolBarLayout->addWidget(mFilterEdit);
    toolBarLayout->addWidget(showWarningsCheckBox);
    toolBarLayout->addWidget(clearButton);
    toolBarLayout->setSpacing(10);

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addLayout(toolBarLayout);
    layout->addWidget(mIssuesView);

    setWidget(widget);

    retranslateUi();
}

void IssuesDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);

    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void IssuesDock::retranslateUi()
{
    setWindowTitle(tr("Issues"));
    mFilterEdit->setPlaceholderText(tr("Filter"));
}

void reportIssue(const Issue &issue)
{
    IssuesModel::instance().addIssue(issue);
}

} // namespace Tiled
