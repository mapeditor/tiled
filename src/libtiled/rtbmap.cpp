/*
 * rtbmap.cpp
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#include "rtbmap.h"

using namespace Tiled;


RTBMap::RTBMap():
            mHasError(false),
            mCustomGlowColor(QColor(mDefaults.defaultValue(CustomGlowColor).toString())),
            mCustomBackgroundColor(QColor(mDefaults.defaultValue(CustomBackgroundColor).toString())),
            mLevelBrightness(mDefaults.defaultValue(LevelBrightness).toDouble()),
            mCloudDensity(mDefaults.defaultValue(CloudDensity).toDouble()),
            mCloudVelocity(mDefaults.defaultValue(CloudVelocity).toDouble()),
            mCloudAlpha(mDefaults.defaultValue(CloudAlpha).toDouble()),
            mSnowDensity(mDefaults.defaultValue(SnowDensity).toDouble()),
            mSnowVelocity(mDefaults.defaultValue(SnowVelocity).toDouble()),
            mSnowRisingVelocity(mDefaults.defaultValue(SnowRisingVelocity).toDouble()),
            mCameraGrain(mDefaults.defaultValue(CameraGrain).toDouble()),
            mCameraContrast(mDefaults.defaultValue(CameraContrast).toDouble()),
            mCameraSaturation(mDefaults.defaultValue(CameraSaturation).toDouble()),
            mCameraGlow(mDefaults.defaultValue(CameraGlow).toDouble()),
            mHasWall(mDefaults.defaultValue(HasWalls).toBool()),
            mLevelName(mDefaults.defaultValue(LevelName).toString()),
            mLevelDescription(mDefaults.defaultValue(LevelDescription).toString()),
            mBackgroundColorScheme(mDefaults.defaultValue(BackgroundColorScheme).toInt()),
            mGlowColorScheme(mDefaults.defaultValue(GlowColorScheme).toInt()),
            mLevelModifier(mDefaults.defaultValue(LevelModifier).toInt()),
            mChapter(mDefaults.defaultValue(Chapter).toInt()),
            mHasStarfield(mDefaults.defaultValue(HasStarfield).toInt()),
            mDifficulty(mDefaults.defaultValue(Difficulty).toInt()),
            mPlayStyle(mDefaults.defaultValue(PlayStyle).toInt()),
            mWorkShopId(0),
            mPreviewImagePath(mDefaults.defaultValue(PreviewImagePath).toString())
{

}
