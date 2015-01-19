/*
 * createcustomobjecttool.h
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


#ifndef CREATECUSTOMOBJECTTOOL_H
#define CREATECUSTOMOBJECTTOOL_H

#include "createobjecttoolinterface.h"
#include "createscalableobjecttool.h"

namespace Tiled {

namespace Internal {

class CreateCustomObjectTool
: public CreateScalableObjectTool {
    Q_OBJECT
public:
    CreateCustomObjectTool(CreateObjectToolInfo* pInfo, QObject *parent);
    void languageChanged();
protected:
    MapObject *createNewMapObject();

private:
    MapObjectFactory mObjFactory;
};

} //Internal

} //Tiled

#endif //CREATECUSTOMOBJECTTOOL_H
