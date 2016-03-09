/*
 * tileseteditor.h
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

#ifndef TILED_INTERNAL_TILESETEDITOR_H
#define TILED_INTERNAL_TILESETEDITOR_H

#include "editor.h"

#include <QHash>

class QMainWindow;
class QStackedWidget;

namespace Tiled {
namespace Internal {

class PropertiesDock;
class TilesetDocument;
class TilesetView;
class TileAnimationEditor;
class TileCollisionEditor;

class TilesetEditor : public Editor
{
    Q_OBJECT

public:
    explicit TilesetEditor(QObject *parent = nullptr);

    void addDocument(Document *document) override;
    void removeDocument(Document *document) override;

    void setCurrentDocument(Document *document) override;
    Document *currentDocument() const override;

    QWidget *editorWidget() const override;

signals:

private slots:
    void currentWidgetChanged();

private:
    QMainWindow *mMainWindow;
    QStackedWidget *mWidgetStack;

    PropertiesDock *mPropertiesDock;
    TileAnimationEditor *mTileAnimationEditor;
    TileCollisionEditor *mTileCollisionEditor;

    QHash<TilesetDocument*, TilesetView*> mViewForTileset;
    TilesetDocument *mCurrentTilesetDocument;
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_TILESETEDITOR_H
