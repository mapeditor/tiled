/*
 * locatorwidget.h
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QFrame>

class QAbstractListModel;

namespace Tiled {

class FilterEdit;
class MatchDelegate;
class MatchesModel;
class ResultsView;

/**
 * Interface for providing a source of locator items based on filter words.
*/
class LocatorSource
{ 
public:
    virtual QString placeholderText() const = 0;
    virtual QAbstractListModel *model() const = 0;
    virtual void setFilterWords(const QStringList &words) = 0;
    virtual void activate(const QModelIndex &index) = 0;
};

class LocatorWidget : public QFrame
{
    Q_OBJECT

public:
    explicit LocatorWidget(std::unique_ptr<LocatorSource> locatorSource,
                           QWidget *parent = nullptr);
    void setVisible(bool visible) override;

private:
    void setFilterText(const QString &text);

    std::unique_ptr<LocatorSource> mLocatorSource;
    FilterEdit *mFilterEdit;
    ResultsView *mResultsView;
    MatchDelegate *mDelegate;
};

class FileMatchesModel;

class FileLocatorSource : public LocatorSource
{
public:
    explicit FileLocatorSource();
    ~FileLocatorSource();

    QString placeholderText() const override;
    QAbstractListModel *model() const override;
    void setFilterWords(const QStringList &words) override;
    void activate(const QModelIndex &index) override;

private:
    std::unique_ptr<FileMatchesModel> mModel;
};

} // namespace Tiled
