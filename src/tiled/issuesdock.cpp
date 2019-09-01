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
#include "issuesmodel.h"
#include "utils.h"

#include <QApplication>
#include <QCheckBox>
#include <QEvent>
#include <QGuiApplication>
#include <QListView>
#include <QPainter>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include <memory>

namespace Tiled {

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

        QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal
                                                                    : QPalette::Disabled;

        if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
            cg = QPalette::Inactive;

        if (opt.state & QStyle::State_Selected)
            painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
        else
            painter->setPen(opt.palette.color(cg, QPalette::Text));

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
    connect(&IssuesModel::instance(), &IssuesModel::counterClicked, this, [this] {
        show();
        raise();
    });

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

} // namespace Tiled
