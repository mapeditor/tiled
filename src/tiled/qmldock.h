/*
 * qmldock.h
 * Copyright 2026, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QObject>
#include <QPointer>
#include <QQmlParserStatus>

class QDockWidget;
class QQmlComponent;

namespace Tiled {

/**
 * A dock widget with Qt Quick based contents, declared by a QML extension.
 *
 * The contents are rendered by a QQuickWidget that shares the script engine,
 * so expressions in the contents can refer to the 'tiled' API. The dock is
 * added to the main window and its placement is persisted based on its
 * mandatory 'name' property, which needs to be unique and stable.
 */
class QmlDock : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(DockArea area READ area WRITE setArea)
    Q_PROPERTY(Window window READ window WRITE setWindow)
    Q_PROPERTY(QQmlComponent *contentItem READ contentItem WRITE setContentItem)
    Q_CLASSINFO("DefaultProperty", "contentItem")

public:
    enum DockArea {
        LeftDockArea = 1,   // synchronized with Qt::DockWidgetArea
        RightDockArea = 2,
        TopDockArea = 4,
        BottomDockArea = 8,
    };
    Q_ENUM(DockArea)

    /**
     * The main window the dock should be added to: either the top-level
     * window (default), or the window of the map or tileset editor.
     */
    enum Window {
        MainWindow,
        MapEditor,
        TilesetEditor,
    };
    Q_ENUM(Window)

    explicit QmlDock(QObject *parent = nullptr);
    ~QmlDock() override;

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }

    QString title() const { return mTitle; }
    void setTitle(const QString &title);

    DockArea area() const { return mArea; }
    void setArea(DockArea area) { mArea = area; }

    Window window() const { return mWindow; }
    void setWindow(Window window) { mWindow = window; }

    QQmlComponent *contentItem() const { return mContentItem; }
    void setContentItem(QQmlComponent *component) { mContentItem = component; }

    void classBegin() override {}
    void componentComplete() override;

signals:
    void titleChanged();

private:
    QString mName;
    QString mTitle;
    DockArea mArea = RightDockArea;
    Window mWindow = MainWindow;
    QQmlComponent *mContentItem = nullptr;
    QPointer<QDockWidget> mDockWidget;
};

} // namespace Tiled
