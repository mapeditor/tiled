/*
 * maintoolbar.h
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindijer.nl>
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

#include <QToolBar>

class QToolButton;

namespace Tiled {
namespace Internal {

class CommandButton;
class Document;

class MainToolBar : public QToolBar
{
    Q_OBJECT

public:
    MainToolBar(QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onOrientationChanged(Qt::Orientation orientation);

    void currentDocumentChanged(Document *document);

private:
    void retranslateUi();

    QToolButton *mNewButton;
    QAction *mOpenAction;
    QAction *mSaveAction;
    QAction *mUndoAction;
    QAction *mRedoAction;
    CommandButton *mCommandButton;
};

} // namespace Internal
} // namespace Tiled
