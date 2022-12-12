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
#include <QTreeView>
#include <QStyledItemDelegate>

namespace Tiled {

class FilterEdit;

static QFont scaledFont(const QFont &font, qreal scale)
{
    QFont scaled(font);
    if (font.pixelSize() > 0)
        scaled.setPixelSize(font.pixelSize() * scale);
    else
        scaled.setPointSizeF(font.pointSizeF() * scale);
    return scaled;
}

class MatchDelegate : public QStyledItemDelegate
{
public:
    MatchDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option,
                  const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    void setWords(const QStringList &words) { mWords = words; }

private:
    class Fonts {
    public:
        Fonts(const QFont &base)
            : small(scaledFont(base, 0.9))
            , big(scaledFont(base, 1.2))
        {}

        const QFont small;
        const QFont big;
    };

    QStringList mWords;
};

class ResultsView : public QTreeView
{
public:
    explicit ResultsView(QWidget *parent = nullptr);

    QSize sizeHint() const override;

    void updateMaximumHeight();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

/**
 * Interface for providing a source of locator items based on filter words.
*/
class LocatorSource
{ 
public:
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

class ProjectFileMatchesModel;

class ProjectFileLocatorSource : public LocatorSource
{
public:
    explicit ProjectFileLocatorSource();
    ~ProjectFileLocatorSource();

    QAbstractListModel *model() const override;
    void setFilterWords(const QStringList &words) override;
    void activate(const QModelIndex &index) override;

private:
    std::unique_ptr<ProjectFileMatchesModel> mModel;
};

} // namespace Tiled
