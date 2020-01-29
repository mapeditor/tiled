/*
 * issuescounter.cpp
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

#include "issuescounter.h"

#include "issuesmodel.h"
#include "utils.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QStyleOption>
#include <QStylePainter>

namespace Tiled {

IssuesCounter::IssuesCounter(QWidget *parent)
    : QAbstractButton(parent)
    , mErrorIcon(new QLabel)
    , mErrorCount(new QLabel)
    , mWarningIcon(new QLabel)
    , mWarningCount(new QLabel)
{
    auto layout = new QHBoxLayout;
    layout->setMargin(Utils::dpiScaled(2));

    int spacing = Utils::dpiScaled(5);
    layout->addSpacing(spacing);
    layout->addWidget(mErrorIcon);
    layout->addWidget(mErrorCount);
    layout->addWidget(mWarningIcon);
    layout->addWidget(mWarningCount);
    layout->addSpacing(spacing);

    setLayout(layout);

    updateLabels();

    const auto &issuesModel = IssuesModel::instance();
    connect(&issuesModel, &IssuesModel::rowsInserted, this, &IssuesCounter::updateLabels);
    connect(&issuesModel, &IssuesModel::rowsRemoved, this, &IssuesCounter::updateLabels);
    connect(&issuesModel, &IssuesModel::modelReset, this, &IssuesCounter::updateLabels);

    connect(this, &QAbstractButton::clicked, &issuesModel, &IssuesModel::counterClicked);
}

void IssuesCounter::paintEvent(QPaintEvent *event)
{
    QStylePainter p(this);

    QStyleOptionButton option;
    option.initFrom(this);
    option.features = underMouse() ? QStyleOptionButton::None : QStyleOptionButton::Flat;
    if (isDown())
        option.state |= QStyle::State_Sunken;
    if (isChecked())
        option.state |= QStyle::State_On;

    p.drawPrimitive(QStyle::PE_PanelButtonCommand, option);

    QWidget::paintEvent(event);
}

void IssuesCounter::updateLabels()
{
    const auto &issuesModel = IssuesModel::instance();
    const int iconSize = Utils::dpiScaled(16);
    const int errorCount = issuesModel.errorCount();
    const int warningCount = issuesModel.warningCount();
    const bool hasErrors = errorCount > 0;
    const bool hasWarnings = warningCount > 0;

    const QFont font = QApplication::font();
    QFont boldFont = font;
    boldFont.setBold(true);

    mErrorCount->setText(QString::number(errorCount));
    mErrorCount->setEnabled(hasErrors);
    mErrorCount->setFont(hasErrors ? boldFont : font);

    mWarningCount->setText(QString::number(warningCount));
    mWarningCount->setEnabled(hasWarnings);
    mWarningCount->setFont(hasWarnings ? boldFont : font);

    const QIcon::Mode errorIconMode = hasErrors ? QIcon::Normal : QIcon::Disabled;
    const QIcon::Mode warningIconMode = hasWarnings ? QIcon::Normal : QIcon::Disabled;

    mErrorIcon->setPixmap(issuesModel.errorIcon().pixmap(iconSize, errorIconMode));
    mWarningIcon->setPixmap(issuesModel.warningIcon().pixmap(iconSize, warningIconMode));

    const QString errorText = tr("%n error(s)", "", errorCount);
    const QString warningText = tr("%n warning(s)", "", warningCount);

    setToolTip(QString(QLatin1String("%1, %2")).arg(errorText, warningText));
}

} // namespace Tiled
