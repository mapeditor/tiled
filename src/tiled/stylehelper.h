/*
 * stylehelper.h
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

#pragma once

#include "tilededitor_global.h"

#include <QFont>
#include <QObject>
#include <QPalette>

#include <optional>

namespace Tiled {

class TILED_EDITOR_EXPORT StyleHelper : public QObject
{
    Q_OBJECT

public:
    static void initialize();
    static StyleHelper *instance() { Q_ASSERT(mInstance); return mInstance; }

    const QString &defaultStyle() { return mDefaultStyle; }
    const QPalette &defaultPalette() { return mDefaultPalette; }

    void applyFont();

signals:
    void styleApplied();

private:
    StyleHelper();

    void apply();

    const QString mDefaultStyle;
    const QPalette mDefaultPalette;
    std::optional<QFont> mDefaultFont;
    const bool mDefaultShowShortcutsInContextMenus = true;

    static StyleHelper *mInstance;
};

} // namespace Tiled
