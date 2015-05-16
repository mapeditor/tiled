/*
 * patreondialog.h
 * Copyright 2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef PATREONDIALOG_H
#define PATREONDIALOG_H

#include <QDialog>

namespace Ui {
class PatreonDialog;
}

namespace Tiled {
namespace Internal {

class PatreonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PatreonDialog(QWidget *parent = 0);
    ~PatreonDialog();

private slots:
    void openPatreonPage();
    void togglePatreonStatus();
    void updatePatreonStatus();

private:
    Ui::PatreonDialog *ui;
};

} // namespace Internal
} // namespace Tiled

#endif // PATREONDIALOG_H
