/*
 * worlddocument.h
 * Copyright 2015-2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

class WorldManager;
class QUndoStack;

#include <QObject>

namespace Tiled {

struct World;

/**
 * Represents an editable world document.
 */
class WorldDocument : public QObject
{
    Q_OBJECT

public:
    WorldDocument(const QString& fileName);
    virtual ~WorldDocument() override;

    QUndoStack* undoStack() const;

protected:
    virtual void onWorldReloaded( const QString& filename );

    QUndoStack* mUndoStack;
    QString mFileName;
};

} // namespace Tiled
