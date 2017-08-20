/*
 * templatesdock.h
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Mohamed Thabet <thabetx@gmail.com>
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
#include <QTreeView>
#include <QAction>

class QAbstractProxyModel;

namespace Tiled {

class ObjectTemplate;

namespace Internal {

class ObjectTemplateModel;

class TemplatesView;

class TemplatesDock : public QDockWidget
{
    Q_OBJECT

public:
    TemplatesDock(QWidget *parent = nullptr);
    ~TemplatesDock();

signals:
    void currentTemplateChanged(ObjectTemplate *objectTemplate);

private slots:
    void newTemplateGroup();
    void openTemplateGroup();

private:
    void retranslateUi();

    TemplatesView *mTemplatesView;

    QAction *mNewTemplateGroup;
    QAction *mOpenTemplateGroup;
};

class TemplatesView : public QTreeView
{
    Q_OBJECT

public:
    QSize sizeHint() const override;
    TemplatesView(QWidget *parent = nullptr);

    void applyTemplateGroups();

signals:
    void currentTemplateChanged(ObjectTemplate *objectTemplate);

private slots:
    void onPressed(const QModelIndex &index);
};

} // namespace Internal
} // namespace Tiled
