/*
 * createcustomobjecttool.cpp
 * Copyright 2015, Chen Zhen <zhen.chen@anansimobile.org>
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

#include "createcustomobjecttool.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreateCustomObjectTool::CreateCustomObjectTool(CreateObjectToolInfo* pInfo, QObject *parent)
    : CreateScalableObjectTool(parent) {
    setName(pInfo->mName);
    setIcon(pInfo->mIcon);
    setShortcut(QKeySequence(pInfo->mShortcut));
    mObjFactory = pInfo->mFactory;

    delete pInfo; //delete pInfo here, pInfo is created in plugin interface.
}

void CreateCustomObjectTool::languageChanged() {
    /* TODO: some way to handle language change. */
}

MapObject* CreateCustomObjectTool::createNewMapObject() {
    if (mObjFactory) {
        return mObjFactory();
    }

    return new MapObject();
}
