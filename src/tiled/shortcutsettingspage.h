/*
 * shortcutsettingspage.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

class QSortFilterProxyModel;

namespace Tiled {

class ActionsModel;
class KeySequenceFilterModel;

namespace Ui {
class ShortcutSettingsPage;
}

class ShortcutSettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ShortcutSettingsPage(QWidget *parent = nullptr);
    ~ShortcutSettingsPage() override;

    QSize sizeHint() const override;

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void refreshConflicts();
    void searchConflicts();

    void importShortcuts();
    void exportShortcuts();

    Ui::ShortcutSettingsPage *ui;
    ActionsModel *mActionsModel;
    KeySequenceFilterModel *mProxyModel;
};

} // namespace Tiled
