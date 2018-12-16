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
#include "tileset.h"

#include <QPointer>

#include <memory>

namespace Tiled {

class ObjectTemplateFormat;

class TILEDSHARED_EXPORT ObjectTemplate : public Object
{
    Q_OBJECT

public:
    ObjectTemplate();
    ObjectTemplate(const QString &fileName);
    ~ObjectTemplate();

    const MapObject *object() const;
    void setObject(const MapObject *object);
    void setObject(std::unique_ptr<MapObject> &&object);

    const QString &fileName() const;
    void setFileName(const QString &fileName);

    void setFormat(ObjectTemplateFormat *format);
    ObjectTemplateFormat *format() const;

    const SharedTileset &tileset() const;

private:
    QString mFileName;
    QPointer<ObjectTemplateFormat> mFormat;
    std::unique_ptr<MapObject> mObject;
    SharedTileset mTileset;
};

inline const MapObject *ObjectTemplate::object() const
{ return mObject.get(); }

inline const QString &ObjectTemplate::fileName() const
{ return mFileName; }

inline void ObjectTemplate::setFileName(const QString &fileName)
{ mFileName = fileName; }

inline const SharedTileset &ObjectTemplate::tileset() const
{ return mTileset; }

} // namespace Tiled
