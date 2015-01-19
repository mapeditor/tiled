/*
 * createobjecttoolinterface.h
 * Copyright 2015, Chen Zhen <zhen.chen@anansimobile.org>
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

#ifndef CREATEOBJECTTOOLINTERFACE_H
#define CREATEOBJECTTOOLINTERFACE_H

#include <QtPlugin>
#include <QStringList>
#include <QIcon>

#include "./mapobject.h"

class QString;

namespace Tiled {

typedef struct {
    QIcon mIcon;
    QString mName;
    QString mShortcut;
    MapObjectFactory mFactory;
} CreateObjectToolInfo;

/**
 * An interface to be implemented to provider info to create an object tool.
 */
class CreateObjectToolInterface
{
public:
    CreateObjectToolInterface() {}
    virtual ~CreateObjectToolInterface() {}

public:
    /**
     * Return a list of CreateObjectToolInfo instance, each instance use to create a "create object tool".
     */
    virtual QList<CreateObjectToolInfo*> getCreateObjectTools() const = 0;
};

} // namespace Tiled

Q_DECLARE_INTERFACE(Tiled::CreateObjectToolInterface,
                    "org.mapeditor.CreateObjectToolInterface")

#endif // CREATEOBJECTTOOLINTERFACE_H
