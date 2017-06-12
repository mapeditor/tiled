/*
 * objecttemplate.h
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Mohamed Thabet <thabetx@gmail.com>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "mapobject.h"
#include "object.h"

namespace Tiled {

class TemplateGroup;

class TILEDSHARED_EXPORT ObjectTemplate : public Object
{
public:
    ObjectTemplate();
    ObjectTemplate(int id, QString name);

    const MapObject *object() const;
    void setObject(MapObject *object);

    int id() const;
    void setId(int id);

    const QString &name() const;
    void setName(const QString &name);

    TemplateGroup *templateGroup() const;
    void setTemplateGroup(TemplateGroup *templateGroup);

private:
    MapObject *mObject;
    int mId;
    QString mName;
    TemplateGroup *mTemplateGroup;
};

inline const MapObject *ObjectTemplate::object() const
{ return mObject; }

inline void ObjectTemplate::setObject(MapObject *object)
{
    mObject = object->clone();
    mObject->setId(0);
}

inline int ObjectTemplate::id() const
{ return mId; }

inline void ObjectTemplate::setId(int id)
{ mId = id; }

inline const QString &ObjectTemplate::name() const
{ return mName; }

inline void ObjectTemplate::setName(const QString &name)
{ mName = name; }

inline TemplateGroup *ObjectTemplate::templateGroup() const
{ return mTemplateGroup; }

inline void ObjectTemplate::setTemplateGroup(TemplateGroup *templateGroup)
{ mTemplateGroup = templateGroup; }

} // namespace Tiled
