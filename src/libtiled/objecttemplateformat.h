/*
 * objecttemplateformat.h
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

#include "mapformat.h"
#include "objecttemplate.h"

namespace Tiled {

class TILEDSHARED_EXPORT ObjectTemplateFormat : public FileFormat
{
    Q_OBJECT

public:
    explicit ObjectTemplateFormat(QObject *parent = nullptr)
        : FileFormat(parent)
    {}

    virtual ObjectTemplate* read(const QString &fileName) = 0;
    virtual bool write(const ObjectTemplate *objectTemplate, const QString &fileName) = 0;
};

TILEDSHARED_EXPORT ObjectTemplate *readObjectTemplate(const QString &fileName,
                                                      QString *error = nullptr);

TILEDSHARED_EXPORT ObjectTemplateFormat *findSupportingTemplateFormat(const QString &fileName);

} // namespace Tiled

Q_DECLARE_INTERFACE(Tiled::ObjectTemplateFormat, "org.mapeditor.ObjectTemplateFormat")
