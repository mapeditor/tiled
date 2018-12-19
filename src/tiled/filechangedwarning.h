/*
 * filechangedwarning.h
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

#include <QWidget>

class QLabel;
class QDialogButtonBox;

namespace Tiled {

class FileChangedWarning : public QWidget
{
    Q_OBJECT

public:
    FileChangedWarning(QWidget *parent = nullptr);

signals:
    void reload();
    void ignore();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *mLabel;
    QDialogButtonBox *mButtons;
};

} // namespace Tiled
