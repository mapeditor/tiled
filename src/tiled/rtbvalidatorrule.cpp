/*
 * rtbvalidatorrule.cpp
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

#include "rtbvalidatorrule.h"

#include <QImage>
#include <QStyle>
#include <QApplication>

using namespace Tiled;
using namespace Tiled::Internal;

RTBValidatorRule::RTBValidatorRule(int type, int ruleID, QString message)
    : mType(type)
    , mRuleID(ruleID)
    , mMessage(message)
    , mMapObject(0)
{
    mWarningIcon = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxWarning).scaled(12,12);
    mErrorIcon = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxCritical).scaled(12,12);
}

QPixmap RTBValidatorRule::symbol()
{
    if(mType == Warning)
        return mWarningIcon;
    else if(mType == Error)
        return mErrorIcon;

    return QPixmap();
}

QString RTBValidatorRule::message()
{
    return tr(" %1").arg(tr(mMessage.toStdString().c_str()));
}

void RTBValidatorRule::setMessage(QString message)
{
    mMessage = message;
}

RTBValidatorRule *RTBValidatorRule::clone()
{
    RTBValidatorRule *rule = new RTBValidatorRule(mType, mRuleID, mMessage);
    return rule;
}

RTBValidatorRule *RTBValidatorRule::cloneWithObject(MapObject *mapObject)
{
    RTBValidatorRule *rule = clone();
    rule->setMapObject(mapObject);
    return rule;
}
