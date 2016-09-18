/*
 * rtbmap.h
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

#ifndef RTBMAP_H
#define RTBMAP_H

#include "rtbmapobject.h"

#include <QColor>
#include <QString>
#include <QCoreApplication>


namespace Tiled {

class RTBMap
{
private:
    // struct with the default values
    struct MapDefaults
    {
        MapDefaults()
            : mMapDefaults(initMapDefaults())
        {
        }

        QVariant defaultValue(int id) const
        {
            if(mMapDefaults.contains(id))
                return mMapDefaults.value(id);

            return QVariant();
        }

    private:
        QHash<int,QVariant> mMapDefaults;

        QHash<int,QVariant> initMapDefaults()
        {
            QHash<int,QVariant> mapDefaults;
            mapDefaults.insert(CustomGlowColor, QColor(255, 255, 255).name());
            mapDefaults.insert(CustomBackgroundColor, QColor(0, 0, 0).name());
            mapDefaults.insert(LevelBrightness, 0.5);
            mapDefaults.insert(CloudDensity, 0.3);
            mapDefaults.insert(CloudVelocity, 0.3);
            mapDefaults.insert(CloudAlpha, 0.3);
            mapDefaults.insert(SnowDensity, 0);
            mapDefaults.insert(SnowVelocity, 0);
            mapDefaults.insert(SnowRisingVelocity, 0);
            mapDefaults.insert(CameraGrain, 0.2);
            mapDefaults.insert(CameraContrast, 0);
            mapDefaults.insert(CameraSaturation, 0.98);
            mapDefaults.insert(CameraGlow, 0.8);
            mapDefaults.insert(HasWalls, true);
            mapDefaults.insert(LevelName, QLatin1String(""));
            mapDefaults.insert(LevelDescription, QCoreApplication::translate("Tiled::RTBMap", "No map description given."));
            mapDefaults.insert(BackgroundColorScheme, 1);
            mapDefaults.insert(LevelModifier, 0);
            mapDefaults.insert(Chapter, 0);
            mapDefaults.insert(HasStarfield, false);
            mapDefaults.insert(GlowColorScheme, 1);
            mapDefaults.insert(Difficulty, 0);
            mapDefaults.insert(PlayStyle, 0);
            mapDefaults.insert(PreviewImagePath, QLatin1String(""));

            return mapDefaults;
        }
    } const mDefaults;

public:
    RTBMap();

    enum ColorScheme {
        CSC,
        CS1,
        CS2,
        CS3,
        CS4
    };

    enum ErrorState {
        Nothing,
        Warning,
        Error
    };

    enum PropertyId {
        Chapter = 25,
        CustomBaseInterval,
        CustomGlowColor,
        CustomBackgroundColor,
        LevelBrightness,
        CloudDensity,
        CloudVelocity,
        CloudAlpha,
        SnowDensity,
        SnowVelocity,
        SnowRisingVelocity,
        CameraGrain,
        CameraContrast,
        CameraSaturation,
        CameraGlow,
        HasWalls,
        CustomMusicTrack,
        LevelName,
        LevelDescription,
        BackgroundColorScheme,
        LevelModifier,
        HasStarfield,
        GlowColorScheme,
        Difficulty,
        PlayStyle,
        PreviewImagePath

    };


    bool hasError() const { return mHasError; }
    void setHasError(bool error) {  mHasError = error; }

    bool hasWarning() const { return mHasWarning; }
    void setHasWarning(bool hasWarning) {  mHasWarning = hasWarning; }

    QColor customGlowColor() const { return mCustomGlowColor; }
    void setCustomGlowColor(const QColor &customGlowColor) {  mCustomGlowColor = customGlowColor; }

    QColor customBackgroundColor() const { return mCustomBackgroundColor; }
    void setCustomBackgroundColor(const QColor &customBackgroundColor) {  mCustomBackgroundColor = customBackgroundColor; }

    double levelBrightness() const { return mLevelBrightness; }
    void setLevelBrightness(double levelBrightness) {  mLevelBrightness = levelBrightness; }

    double cloudDensity() const { return mCloudDensity; }
    void setCloudDensity(double cloudDensity) {  mCloudDensity = cloudDensity; }

    double cloudVelocity() const { return mCloudVelocity; }
    void setCloudVelocity(double cloudVelocity){  mCloudVelocity = cloudVelocity; }

    double cloudAlpha() const { return mCloudAlpha; }
    void setCloudAlpha(double cloudAlpha){  mCloudAlpha = cloudAlpha; }

    double snowDensity() const { return mSnowDensity; }
    void setSnowDensity(double snowDensity){  mSnowDensity = snowDensity; }

    double snowVelocity() const { return mSnowVelocity; }
    void setSnowVelocity(double snowVelocity){  mSnowVelocity = snowVelocity; }

    double snowRisingVelocity() const { return mSnowRisingVelocity; }
    void setSnowRisingVelocity(double snowRisingVelocity){  mSnowRisingVelocity = snowRisingVelocity; }

    double cameraGrain() const { return mCameraGrain; }
    void setCameraGrain(double cameraGrain){  mCameraGrain = cameraGrain; }

    double cameraContrast() const { return mCameraContrast; }
    void setCameraContrast(double cameraContrast){  mCameraContrast = cameraContrast; }

    double cameraSaturation() const { return mCameraSaturation; }
    void setCameraSaturation(double cameraSaturation){  mCameraSaturation = cameraSaturation; }

    double cameraGlow() const { return mCameraGlow; }
    void setCameraGlow(double cameraGlow){  mCameraGlow = cameraGlow; }

    bool hasWall() const { return mHasWall; }
    void setHasWall(bool hasWall)
    {
        if(mHasWall == hasWall)
            return;

        mHasWall = hasWall;
    }

    QString levelName() const { return mLevelName; }
    void setLevelName(const QString &levelName){  mLevelName = levelName; }

    QString levelDescription() const { return mLevelDescription; }
    void setLevelDescription(const QString &levelDescription){  mLevelDescription = levelDescription; }

    int backgroundColorScheme() const { return mBackgroundColorScheme; }
    void setBackgroundColorScheme(const int backgroundColorScheme){  mBackgroundColorScheme = backgroundColorScheme; }

    int glowColorScheme() const { return mGlowColorScheme; }
    void setGlowColorScheme(const int glowColorScheme){  mGlowColorScheme = glowColorScheme; }

    int levelModifier() const { return mLevelModifier; }
    void setLevelModifier(const int levelModifier){  mLevelModifier = levelModifier; }

    int chapter() const { return mChapter; }
    void setChapter(int chapter) {  mChapter = chapter; }

    bool hasStarfield() const { return mHasStarfield; }
    void setHasStarfield(bool hasStarfield)
    {
        if(mHasStarfield == hasStarfield)
            return;

        mHasStarfield = hasStarfield;
    }

    int difficulty() const { return mDifficulty; }
    void setDifficulty(int difficulty) {  mDifficulty = difficulty; }

    int playStyle() const { return mPlayStyle; }
    void setPlayStyle(int playStyle) {  mPlayStyle = playStyle; }

    QColor nameColor(PropertyId id) const
    {
        switch (mPropertyErrorState[id]) {
        case Error:
            return Qt::red;
            break;
        case Warning:
            return RTBMapObject::warningColor();
            break;
        case Nothing:
        default:
            return Qt::black;
            break;
        }
    }

    void setPropertyErrorState(PropertyId id, ErrorState state)
    {
        if(state == Error)
        {
            mPropertyErrorState[id] = state;
            mHasError = true;
        }
        else if(state == Warning && mPropertyErrorState[id] != Error)
        {
            mPropertyErrorState[id] = state;
            mHasWarning = true;
        }
    }

    int propertyErrorState(PropertyId id) const { return mPropertyErrorState[id]; }
    void clearPropertyErrorState()
    {
        QHash<PropertyId, int>::iterator i;
        for (i = mPropertyErrorState.begin(); i != mPropertyErrorState.end(); ++i)
            i.value() = 0;

        mHasError = false;
        mHasWarning = false;
    }

    QVariant defaultValue(int id)
    {
        return mDefaults.defaultValue(id);
    }


    int workShopId() const { return mWorkShopId; }
    void setWorkShopId(int workShopId) { mWorkShopId = workShopId;}

    QString previewImagePath() const { return mPreviewImagePath; }
    void setPreviewImagePath(QString previewImagePath) { mPreviewImagePath = previewImagePath;}

private:
    bool mHasError;
    bool mHasWarning;
    QColor mCustomGlowColor;
    QColor mCustomBackgroundColor;
    double mLevelBrightness;
    double mCloudDensity;
    double mCloudVelocity;
    double mCloudAlpha;
    double mSnowDensity;
    double mSnowVelocity;
    double mSnowRisingVelocity;
    double mCameraGrain;
    double mCameraContrast;
    double mCameraSaturation;
    double mCameraGlow;
    bool mHasWall;
    QString mLevelName;
    QString mLevelDescription;
    int mBackgroundColorScheme;
    int mGlowColorScheme;
    int mLevelModifier;
    int mChapter;
    bool mHasStarfield;
    int mDifficulty;
    int mPlayStyle;
    int mWorkShopId;
    QString mPreviewImagePath;

    QHash<PropertyId, int> mPropertyErrorState;

};


}
#endif // RTBMAP_H
