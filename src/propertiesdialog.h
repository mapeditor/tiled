/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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

#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>

namespace Ui {
class PropertiesDialog;
}

namespace Tiled {
namespace Internal {

class PropertiesModel;

class PropertiesDialog : public QDialog
{
    public:
        /**
         * Constructor.
         */
        PropertiesDialog(QWidget *parent = 0);

        /**
         * Sets the properties displayed by this dialog.
         */
        void setProperties(QMap<QString, QString> properties);

    private:
        Ui::PropertiesDialog *mUi;
        PropertiesModel *mModel;
};

} // namespace Internal
} // namespace Tiled

#endif // PROPERTIESDIALOG_H
