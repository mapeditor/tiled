/*
 * flexiblescrollbar.cpp
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 * Copyright 2016, Mamed Ibrahimov <ibramlab@gmail.com>
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

#include "flexiblescrollbar.h"

namespace Tiled {
namespace Internal {

FlexibleScrollBar::FlexibleScrollBar(Qt::Orientation orientation, QWidget *parent)
    : QScrollBar(orientation, parent)
    , mOverrideMinimum(0)
    , mOverrideMaximum(0)
    , mDesiredMinimum(0)
    , mDesiredMaximum(0)
    , mAllowRangeChange(false)
{}

void FlexibleScrollBar::forceSetValue(int value)
{
    if (value < minimum())
        setOverrideMinimum(value);
    if (value > maximum())
        setOverrideMaximum(value);

    setValue(value);
}

void FlexibleScrollBar::allowNextRangeChange()
{
    mAllowRangeChange = true;
}

void FlexibleScrollBar::sliderChange(QAbstractSlider::SliderChange change)
{
    switch (change) {
    case SliderRangeChange:
        if (!mAllowRangeChange) {
            int min = minimum();
            int max = maximum();
            int val = value();

            mDesiredMinimum = min;
            mDesiredMaximum = max;

            if (min == 0 && max == 0) {
                // view is resetting the scroll bar...
                mOverrideMinimum = min;
                mOverrideMaximum = max;
            } else if (min > val || max < val) {
                // shrink only as much as allowed by the current value
                allowNextRangeChange();
                setRange(qMin(mDesiredMinimum, val),
                         qMax(mDesiredMaximum, val));
                return;
            }
        }
        mAllowRangeChange = false;
        break;
    case SliderValueChange:
        // shrink range back to desired range as much as possible
        if (mOverrideMinimum < mDesiredMinimum)
            setOverrideMinimum(qMin(mDesiredMinimum, value()));
        if (mOverrideMaximum > mDesiredMaximum)
            setOverrideMaximum(qMax(mDesiredMaximum, value()));
        break;
    case SliderOrientationChange:
    case SliderStepsChange:
        break;
    }

    QScrollBar::sliderChange(change);
}

void FlexibleScrollBar::setOverrideMinimum(int min)
{
    mOverrideMinimum = min;
    allowNextRangeChange();
    setMinimum(min);
}

void FlexibleScrollBar::setOverrideMaximum(int max)
{
    mOverrideMaximum = max;
    allowNextRangeChange();
    setMaximum(max);
}

} // namespace Internal
} // namespace Tiled
