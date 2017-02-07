/*
 * flexiblescrollbar.h
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QScrollBar>

namespace Tiled {
namespace Internal {

class FlexibleScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    FlexibleScrollBar(Qt::Orientation orientation, QWidget *parent = nullptr);

    void forceSetValue(int value);

    void allowNextRangeChange();

protected:
    void sliderChange(SliderChange change) override;

private:
    void setOverrideMinimum(int min);
    void setOverrideMaximum(int max);

    int mOverrideMinimum;
    int mOverrideMaximum;
    int mDesiredMinimum;
    int mDesiredMaximum;
    bool mAllowRangeChange;
};

} // namespace Internal
} // namespace Tiled
