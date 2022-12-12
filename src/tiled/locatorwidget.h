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

struct LocatorMatch
{
    int score;
    int offset;
    QString path;

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QStringRef relativePath() const { return path.midRef(offset); }
#else
    QStringView relativePath() const { return QStringView(path).mid(offset); }
#endif
};

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

/*
 * Base class of the model of entities matching 
 * the filter words the user enters.
*/
class LocatorMatchesModel: public QAbstractListModel
{
public:
    explicit LocatorMatchesModel(QObject *parent =nullptr);
    const QVector<LocatorMatch> &matches() const { return mMatches; }
    virtual void setMatches(QVector<LocatorMatch> matches) = 0;     
private:
    QVector<LocatorMatch> mMatches;
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
 * Base class that provides an implementation of searching for
 * and clicking on items in locator widgets.
 * 
 * Whenever the filter words changed, the signal filterWordsChanged
 * will be triggered.
*/
class LocatorSource: public QObject
{ 
Q_OBJECT
public:
    LocatorSource(QObject *parent = nullptr);
    virtual void setFilterWords(const QString &text) = 0;
    virtual void activate(const QModelIndex &index) = 0;
    MatchDelegate *delegate;
    LocatorMatchesModel *listModel;
signals:
    void filterWordsChanged();
};
class LocatorWidget : public QFrame
{
    Q_OBJECT

public:
    explicit LocatorWidget(LocatorSource *locatorSource, QWidget *parent = nullptr);
    LocatorMatchesModel *listModel;
    void setVisible(bool visible) override;

protected:
    LocatorSource *mLocatorSource;

    /* Called to adjust the size of the widget when
     * the filter words change.
     */
    void adjustLayout();
private:
    FilterEdit *mFilterEdit;
    ResultsView *mResultsView;

};


class ProjectFileLocatorSource: public LocatorSource {
    public:
        explicit ProjectFileLocatorSource(QObject *parent);
        void setFilterWords(const QString &text) override;
        void activate(const QModelIndex &index) override;
        LocatorMatchesModel *listModel;
        MatchDelegate *delegate;
};
} // namespace Tiled
