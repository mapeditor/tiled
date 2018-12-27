/*
 * flexiblescrollbar.cpp
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

#include "flexiblescrollbar.h"

namespace Tiled {

FlexibleScrollBar::FlexibleScrollBar(Qt::Orientation orientation, QWidget *parent)
    : QScrollBar(orientation, parent)
    , mOverrideMinimum(0)
    , mOverrideMaximum(0)
    , mDesiredMinimum(0)
    , mDesiredMaximum(0)
    , mInternalRangeChange(false)
{}

void FlexibleScrollBar::forceSetValue(int value)
{
    setOverrideRange(std::min(value, minimum()),
                     std::max(value, maximum()));
    setValue(value);
}

void FlexibleScrollBar::sliderChange(QAbstractSlider::SliderChange change)
{
    switch (change) {
    case SliderRangeChange:
        if (!mInternalRangeChange) {
            // The range change was not explicitly allowed, so it is based on
            // some external initiative. This means we may have to set back the
            // range, to allow the current value of the slider to remain valid.

            int min = minimum();
            int max = maximum();
            int val = value();

            // remember the range that was desired by the external source
            mDesiredMinimum = min;
            mDesiredMaximum = max;

            if (min == 0 && max == 0) {
                // view is resetting the scroll bar...
                mOverrideMinimum = min;
                mOverrideMaximum = max;
            } else if (val < min) {
                // raise minimum only as much as allowed by the current value
                setOverrideRange(val, max);
                return;
            } else if (val > max) {
                // lower maximum only as much as allowed by the current value
                setOverrideRange(min, val);
                return;
            } else {
                // range change allowed because value stays valid
                mOverrideMinimum = min;
                mOverrideMaximum = max;
            }
        }
        break;
    case SliderValueChange:
        // shrink range back to desired range as much as possible
        setOverrideRange(std::min(mDesiredMinimum, value()),
                         std::max(mDesiredMaximum, value()));
        break;
    case SliderOrientationChange:
    case SliderStepsChange:
        break;
    }

    QScrollBar::sliderChange(change);
}

void FlexibleScrollBar::setOverrideRange(int min, int max)
{
    mOverrideMinimum = min;
    mOverrideMaximum = max;
    mInternalRangeChange = true;
    setRange(min, max);
    mInternalRangeChange = false;
}

} // namespace Tiled
