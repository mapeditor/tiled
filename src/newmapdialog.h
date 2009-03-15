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

#ifndef NEWMAPDIALOG_H
#define NEWMAPDIALOG_H

#include <QDialog>

namespace Ui {
class NewMapDialog;
}

namespace Tiled {
namespace Internal {

class NewMapDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(NewMapDialog)

public:
    /**
     * Constructor.
     */
    NewMapDialog(QWidget *parent = 0);

    /**
     * Destructor.
     */
    ~NewMapDialog();

    int mapWidth() const;
    int mapHeight() const;
    int tileWidth() const;
    int tileHeight() const;

private:
    Ui::NewMapDialog *mUi;
};

} // namespace Internal
} // namespace Tiled

#endif // NEWMAPDIALOG_H
