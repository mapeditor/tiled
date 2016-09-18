/*
 * rtbvalidatorrule.h
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

#ifndef RTBVALIDATORRULE_H
#define RTBVALIDATORRULE_H

namespace Tiled {

namespace Internal {


class RTBValidatorRule : public QObject
{
    Q_OBJECT

public:
    RTBValidatorRule(int type, int ruleID, QString message);

    enum Type{
        Warning,
        Error
    };

    enum RuleID{
        // Error
        StartLocation,
        FinishHole,
        TeleporterMissingTarget,
        LaserBeamRotation,
        CameraTriggerMissingTarget,
        LaserBeamOnWall,
        LaserBeamBlocked,
        ObjectOnFloor,
        ObjectOverlaid,
        FinishHoleSurrounded,
        ObjectInWall,
        WallsAllowed,
        ObjectOnGround,
        NPCBallSpawnerWithWalls,
        DifficultySet,
        PlayStyleSet,
        PreviewImageSize,
        ProjectileTurretBlocked,
        // Warning
        ButtonTarget,
        LaserBeamStartEndDegree,
        ObjectOverlap,
        DefaultValues,
        TextMissing
    };

    QPixmap symbol();
    QString message();
    void setMessage(QString message);
    int ruleID() { return mRuleID; }

    MapObject *mapObject() { return mMapObject; }
    void setMapObject(MapObject *mapObject) { mMapObject = mapObject; }

    RTBValidatorRule *clone();
    RTBValidatorRule *cloneWithObject(MapObject *mapObject);

private:
    int mType;
    int mRuleID;
    QString mMessage;
    QPixmap mWarningIcon;
    QPixmap mErrorIcon;
    MapObject *mMapObject;
};

} // namespace Internal
} // namespace Tiled

#endif // RTBVALIDATORRULE_H
