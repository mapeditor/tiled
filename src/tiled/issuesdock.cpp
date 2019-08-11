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
#include <QApplication>
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

#include <memory>

namespace Tiled {

Issue::Issue(Issue::Severity severity,
             const QString &text)
    : mSeverity(severity)
    , mText(text)
{
}

void Issue::setCallback(std::function<void ()> callback, void *context)
{
    mCallback = std::move(callback);
    mContext = context;
}

void Issue::addOccurrence(const Issue &issue)
{
    mOccurrences += 1;
    setCallback(issue.callback(), issue.context());
}


class IssuesModel : public QAbstractListModel
{
public:
    enum {
        IssueRole = Qt::UserRole
    };

    static IssuesModel &instance();

    unsigned addIssue(const Issue &issue);
    void removeIssues(const QList<unsigned> &issueIds);
    void removeIssuesWithContext(void *context);
    void clear();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    IssuesModel(QObject *parent = nullptr);

    void removeIssues(const RangeSet<int> &indexes);

    QVector<Issue> mIssues;

    QIcon mErrorIcon;
    QIcon mWarningIcon;

    static unsigned mNextIssueId;
};

unsigned IssuesModel::mNextIssueId = 1;

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

unsigned IssuesModel::addIssue(const Issue &issue)
{
    int i = mIssues.indexOf(issue);
    if (i != -1) {
        auto &existingIssue = mIssues[i];
        existingIssue.addOccurrence(issue);

        QModelIndex modelIndex = index(i);
        emit dataChanged(modelIndex, modelIndex);

        return existingIssue.id();
    }

    const_cast<Issue&>(issue).mId = mNextIssueId++;

    beginInsertRows(QModelIndex(), mIssues.size(), mIssues.size());
    mIssues.append(issue);
    endInsertRows();

    return issue.id();
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

void IssuesModel::removeIssuesWithContext(void *context)
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
        beginRemoveRows(QModelIndex(), it.first(), it.last());
        mIssues.remove(it.first(), it.length());
        endRemoveRows();
    } while (it != begin);
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
            if (issue.severity() == Issue::Warning)
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
    Q_ASSERT(index.isValid());

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const Issue issue = index.data(IssuesModel::IssueRole).value<Issue>();
    const bool isDark = opt.palette.base().color().value() <= 128;

    QColor textColor;

    switch (issue.severity()) {
    case Issue::Error:
        textColor = isDark ? QColor(255, 55, 55) : QColor(164, 0, 15);
        break;
    case Issue::Warning:
        textColor = isDark ? QColor(255, 183, 0) : QColor(113, 81, 0);
        break;
    }

    opt.palette.setColor(QPalette::Text, textColor);

    const QWidget *widget = opt.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();

    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    // Add lines between issues
    auto color = textColor;
    color.setAlpha(32);
    painter->setPen(color);
    painter->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());

    // Add occurrences label
    const int occurrences = index.data(IssuesModel::IssueRole).value<Issue>().occurrences();
    if (occurrences > 1) {
        auto smallFont = opt.font;
        if (smallFont.pixelSize() > 0)
            smallFont.setPixelSize(smallFont.pixelSize() * 0.9);
        else
            smallFont.setPointSizeF(smallFont.pointSizeF() * 0.9);

        painter->setPen(textColor);
        painter->setFont(smallFont);
        painter->drawText(opt.rect.adjusted(Utils::dpiScaled(4), 0, Utils::dpiScaled(-4), 0),
                          QString(QLatin1String("(%1)")).arg(occurrences),
                          QStyle::visualAlignment(opt.direction, Qt::AlignRight | Qt::AlignVCenter));
    }
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

    connect(mIssuesView, &QAbstractItemView::activated, this, &IssuesDock::activated);

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

void IssuesDock::activated(const QModelIndex &index)
{
    const auto issue = mProxyModel->data(index, IssuesModel::IssueRole).value<Issue>();
    if (issue.callback())
        issue.callback()();
}

void IssuesDock::retranslateUi()
{
    setWindowTitle(tr("Issues"));
    mFilterEdit->setPlaceholderText(tr("Filter"));
}

unsigned reportIssue(const Issue &issue)
{
    return IssuesModel::instance().addIssue(issue);
}

void clearIssues(const QList<unsigned> &issueIds)
{
    IssuesModel::instance().removeIssues(issueIds);
}

void clearIssuesWithContext(void *context)
{
    IssuesModel::instance().removeIssuesWithContext(context);
}

} // namespace Tiled
