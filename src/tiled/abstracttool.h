/*
 * abstracttool.h
 * Copyright 2009-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "changeevents.h"
#include "id.h"
#include "layer.h"

#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QIcon>
#include <QKeySequence>
#include <QMetaType>
#include <QObject>
#include <QString>

class QEvent;
class QKeyEvent;
class QToolBar;
class QUndoStack;

namespace Tiled {

class Layer;
class Tile;
class ObjectTemplate;

class MapDocument;
class MapScene;
class ToolManager;

/**
 * An abstraction of any kind of tool used to edit the map.
 *
 * Events that hit the MapScene are forwarded to the current tool, which can
 * handle them as appropriate for that tool.
 *
 * A tool will usually add one or more QGraphicsItems to the scene in order to
 * represent it to the user.
 */
class AbstractTool : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QByteArray id READ idName CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon NOTIFY changed)
    Q_PROPERTY(QKeySequence shortcut READ shortcut WRITE setShortcut NOTIFY changed)
    Q_PROPERTY(QString statusInfo READ statusInfo WRITE setStatusInfo NOTIFY statusInfoChanged)
    Q_PROPERTY(QCursor cursor READ cursor WRITE setCursor NOTIFY cursorChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(bool usesSelectedTiles READ usesSelectedTiles WRITE setUsesSelectedTiles)
    Q_PROPERTY(bool usesWangSets READ usesWangSets WRITE setUsesWangSets)
    Q_PROPERTY(int targetLayerType READ targetLayerType WRITE setTargetLayerType)

public:
    /**
     * Constructs an abstract tool with the given \a name and \a icon.
     */
    AbstractTool(Id id,
                 const QString &name,
                 const QIcon &icon,
                 const QKeySequence &shortcut,
                 QObject *parent = nullptr);

    Id id() const;
    QByteArray idName() const;

    QString name() const;
    void setName(const QString &name);

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    QKeySequence shortcut() const;
    void setShortcut(const QKeySequence &shortcut);

    QString statusInfo() const;
    void setStatusInfo(const QString &statusInfo);

    QCursor cursor() const;
    void setCursor(const QCursor &cursor);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isVisible() const;
    void setVisible(bool visible);

    bool usesSelectedTiles() const;
    void setUsesSelectedTiles(bool usesSelectedTiles);

    bool usesWangSets() const;
    void setUsesWangSets(bool usesWangSets);

    int targetLayerType() const;
    void setTargetLayerType(int targetLayerType);

    ToolManager *toolManager() const;
    Tile *tile() const;
    ObjectTemplate *objectTemplate() const;

    /**
     * Activates this tool. If the tool plans to add any items to the scene, it
     * probably wants to do it here.
     */
    virtual void activate(MapScene *scene);

    /**
     * Deactivates this tool. Should do any necessary cleanup to make sure the
     * tool is no longer active.
     */
    virtual void deactivate(MapScene *scene);

    virtual void keyPressed(QKeyEvent *);

    /**
     * Called when the mouse entered the scene. This is usually an appropriate
     * time to make a hover item visible.
     */
    virtual void mouseEntered() = 0;

    /**
     * Called when the mouse left the scene.
     */
    virtual void mouseLeft() = 0;

    /**
     * Called when the mouse cursor moves in the scene.
     */
    virtual void mouseMoved(const QPointF &pos,
                            Qt::KeyboardModifiers modifiers) = 0;

    /**
     * Called when a mouse button is pressed on the scene.
     */
    virtual void mousePressed(QGraphicsSceneMouseEvent *event) = 0;

    /**
     * Called when a mouse button is released on the scene.
     */
    virtual void mouseReleased(QGraphicsSceneMouseEvent *event) = 0;

    /**
     * Called when a mouse button is pressed a second time on the scene, after
     * a short interval.
     *
     * By default, this function calls mousePressed.
     */
    virtual void mouseDoubleClicked(QGraphicsSceneMouseEvent *event);

    /**
     * Called when the user presses or releases a modifier key resulting
     * in a change of modifier status, and after the tool is activated.
     */
    virtual void modifiersChanged(Qt::KeyboardModifiers) {}

    /**
     * Called when the application language changed.
     */
    virtual void languageChanged() = 0;

    virtual void populateToolBar(QToolBar*) {}

    void setMapDocument(MapDocument *mapDocument);

    /**
     * override to use a different undo stack than the one from the document.
     */
    virtual QUndoStack* undoStack() { return nullptr; }

protected:
    virtual void changeEvent(const ChangeEvent &event);

    /**
     * Can be used to respond to the map document changing.
     */
    virtual void mapDocumentChanged(MapDocument *oldDocument,
                                    MapDocument *newDocument)
    {
        Q_UNUSED(oldDocument)
        Q_UNUSED(newDocument)
    }

    MapDocument *mapDocument() const { return mMapDocument; }
    MapScene *mapScene() const { return mMapScene; }

    Layer *currentLayer() const;

    /**
     * By default, this function is called after the current map has changed
     * and when the current layer changes. It can be overridden to implement
     * custom logic for when the tool should be enabled.
     *
     * The default implementation enables tools when a map document is set.
     */
    virtual void updateEnabledState();

signals:
    void changed();
    void statusInfoChanged(const QString &statusInfo);
    void cursorChanged(const QCursor &cursor);
    void enabledChanged(bool enabled);
    void visibleChanged(bool visible);

private:
    friend class ToolManager;

    QString mName;
    QIcon mIcon;
    QKeySequence mShortcut;
    QString mStatusInfo;
    QCursor mCursor;
    Id mId;
    bool mEnabled = false;
    bool mVisible = true;
    bool mUsesSelectedTiles = false;
    bool mUsesWangSets = false;
    int mTargetLayerType = 0;

    ToolManager *mToolManager = nullptr;
    MapDocument *mMapDocument = nullptr;
    MapScene *mMapScene = nullptr;
};


inline Id AbstractTool::id() const
{
    return mId;
}

inline QByteArray AbstractTool::idName() const
{
    return mId.name();
}

inline QString AbstractTool::name() const
{
    return mName;
}

inline QIcon AbstractTool::icon() const
{
    return mIcon;
}

inline QKeySequence AbstractTool::shortcut() const
{
    return mShortcut;
}


inline QString AbstractTool::statusInfo() const
{
    return mStatusInfo;
}

inline QCursor AbstractTool::cursor() const
{
    return mCursor;
}

inline bool AbstractTool::isEnabled() const
{
    return mEnabled;
}

inline bool AbstractTool::isVisible() const
{
    return mVisible;
}

inline bool AbstractTool::usesSelectedTiles() const
{
    return mUsesSelectedTiles;
}

inline void AbstractTool::setUsesSelectedTiles(bool usesSelectedTiles)
{
    mUsesSelectedTiles = usesSelectedTiles;
}

inline bool AbstractTool::usesWangSets() const
{
    return mUsesWangSets;
}

inline void AbstractTool::setUsesWangSets(bool usesWangSets)
{
    mUsesWangSets = usesWangSets;
}

inline int AbstractTool::targetLayerType() const
{
    return mTargetLayerType;
}

/**
 * Returns the ToolManager with which this tool is registered, if any.
 */
inline ToolManager *AbstractTool::toolManager() const
{
    return mToolManager;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::AbstractTool*)
