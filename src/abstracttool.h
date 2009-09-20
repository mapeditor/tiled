/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef ABSTRACTTOOL_H
#define ABSTRACTTOOL_H

#include <QObject>
#include <QIcon>
#include <QMetaType>
#include <QString>

class QEvent;
class QGraphicsSceneMouseEvent;

namespace Tiled {
namespace Internal {

class MapScene;

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

public:
    /**
     * Constructs an abstract tool with the given \a name and \a icon.
     */
    AbstractTool(const QString &name, const QIcon &icon, QObject *parent = 0)
        : QObject(parent)
        , mName(name)
        , mIcon(icon)
    {}

    virtual ~AbstractTool() {}

    QString name() const { return mName; }
    QIcon icon() const { return mIcon; }

    /**
     * Enables this tool. If the tool plans to add any items to the scene, it
     * probably wants to do it here.
     */
    virtual void enable(MapScene *scene) = 0;

    /**
     * Disables this tool. Should do any necessary cleanup to make sure the
     * tool is no longer active.
     */
    virtual void disable() = 0;

    virtual void enterEvent(QEvent *event) = 0;
    virtual void leaveEvent(QEvent *event) = 0;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) = 0;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) = 0;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) = 0;

private:
    QString mName;
    QIcon mIcon;
};

} // namespace Internal
} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Internal::AbstractTool*)

#endif // ABSTRACTTOOL_H
