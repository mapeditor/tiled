/*
 * propertiesdock.h
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QDockWidget>

class QTabBar;

namespace Tiled {

class Document;

class PropertiesWidget;

class PropertiesDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit PropertiesDock(QWidget *parent = nullptr);

    /**
     * Sets the \a document on which this properties dock will act.
     */
    void setDocument(Document *document);

public slots:
    void selectCustomProperty(const QString &name);

protected:
    bool event(QEvent *event) override;

private:
    enum TabType {
        MapTab,
        LayerTab,
        MapObjectTab,
        TilesetTab,
        TileTab,
        WangSetTab,
        WangColorTab,
    };

    void bringToFront();
    void retranslateUi();

    void updateTabs();
    void tabChanged(int index);

    QTabBar *mTabBar;
    PropertiesWidget *mPropertiesWidget;
    Document *mDocument = nullptr;
};

} // namespace Tiled
