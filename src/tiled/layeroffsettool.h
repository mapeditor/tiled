/*
* layeroffsettool.h
* Copyright 2014, Mattia Basaglia
* Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "abstracttool.h"

namespace Tiled {

class LayerOffsetTool : public AbstractTool
{
    Q_OBJECT
    Q_INTERFACES(Tiled::AbstractTool)

public:
    explicit LayerOffsetTool(QObject *parent = nullptr);

    void activate(MapScene *) override;
    void deactivate(MapScene *) override;
    void keyPressed(QKeyEvent *) override;
    void mouseEntered() override;
    void mouseLeft() override;
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;
    void modifiersChanged(Qt::KeyboardModifiers) override;

    void languageChanged() override;

protected slots:
    void updateEnabledState() override;

protected:
    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument) override;

private:
    void startDrag(const QPointF &pos);
    void abortDrag();
    void finishDrag();

    struct DraggingLayer {
        Layer *layer;
        QPointF oldOffset;
    };

    bool mMousePressed;
    bool mDragging;
    bool mApplyingChange;
    QPoint mMouseScreenStart;
    QPointF mMouseSceneStart;
    QVector<DraggingLayer> mDraggingLayers;
};

} // namespace Tiled
