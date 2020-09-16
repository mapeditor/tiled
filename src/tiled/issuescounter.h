/*
 * issuescounter.h
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

#include <QToolButton>

class QLabel;

namespace Tiled {

class IssuesCounter : public QToolButton
{
    Q_OBJECT

public:
    explicit IssuesCounter(QWidget *parent = nullptr);

    // Skip the QToolButton implementations, due to custum contents
    QSize sizeHint() const override { return QAbstractButton::sizeHint(); }
    QSize minimumSizeHint() const override { return QAbstractButton::minimumSizeHint(); };

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void updateLabels();

    QLabel *mErrorIcon;
    QLabel *mErrorCount;
    QLabel *mWarningIcon;
    QLabel *mWarningCount;
};

} // namespace Tiled
