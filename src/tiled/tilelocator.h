/*
 * tilelocator.h
 * Copyright 2025, Siraj Ahmadzai
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

#include "locatorwidget.h"

namespace Tiled {

class TileMatchDelegate;

class TileLocatorSource : public LocatorSource
{
    Q_OBJECT

public:
    explicit TileLocatorSource(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // LocatorSource
    QAbstractItemDelegate *delegate() const override;
    QString placeholderText() const override;
    void setFilterWords(const QStringList &words) override;
    void activate(const QModelIndex &index) override;

private:
    struct Match {
        QPoint tilePos;
        QString displayText;
        QString description;
    };

    TileMatchDelegate *mDelegate;
    QVector<Match> mMatches;

    static QVector<Match> parseCoordinates(const QStringList &words);
    static bool parseCoordinate(const QString &text, QPoint &tilePos);
};

} // namespace Tiled 