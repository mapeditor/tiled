/*
 * actionmanager.h
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#ifndef TILED_INTERNAL_ACTIONMANAGER_H
#define TILED_INTERNAL_ACTIONMANAGER_H

#include "id.h"

#include <QObject>

class QAction;

namespace Tiled {
namespace Internal {

class MainWindow;

/**
 * Manager of global actions.
 */
class ActionManager : public QObject
{
    Q_OBJECT

public:
    static void registerAction(QAction *action, Id id);

    static QAction *action(Id id);

signals:
    void actionAdded(Id id);

private:
    explicit ActionManager(QObject *parent = nullptr);
    ~ActionManager();

    friend class Tiled::Internal::MainWindow;   // creation
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_ACTIONMANAGER_H
