/*
 * editor.h
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

#include <QObject>

class QToolBar;
class QDockWidget;

namespace Tiled {
namespace Internal {

class Document;
class Zoomable;

class Editor : public QObject
{
    Q_OBJECT

public:
    enum StandardAction {
        CutAction           = 0x01,
        CopyAction          = 0x02,
        PasteAction         = 0x04,
        PasteInPlaceAction  = 0x08,
        DeleteAction        = 0x10
    };
    Q_DECLARE_FLAGS(StandardActions, StandardAction)
    Q_FLAGS(StandardActions)

    explicit Editor(QObject *parent = nullptr);

    virtual void saveState() = 0;
    virtual void restoreState() = 0;

    virtual void addDocument(Document *document) = 0;
    virtual void removeDocument(Document *document) = 0;

    virtual void setCurrentDocument(Document *document) = 0;
    virtual Document *currentDocument() const = 0;

    virtual QWidget *editorWidget() const = 0;
    virtual Zoomable *zoomable() const = 0;

    virtual QList<QToolBar*> toolBars() const = 0;
    virtual QList<QDockWidget*> dockWidgets() const = 0;

    virtual StandardActions enabledStandardActions() const = 0;
    virtual void performStandardAction(StandardAction action) = 0;

    virtual void resetLayout() = 0;

signals:
    void enabledStandardActionsChanged();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Editor::StandardActions)

} // namespace Internal
} // namespace Tiled
