/*
 * properties.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "properties.h"
#include "mapobject.h"

using namespace Tiled;

Property::Property()
{
    mType = PropertyType_Invalid;
}

bool Property::IsValid()
{
    return mType != PropertyType_Invalid;
}

QString Property::TypeAsQString() const
{
    return Property::PropertyTypeToQString(mType);
}

QString Property::PropertyTypeToQString(PropertyType type)
{
    switch(type)
    {
        case PropertyType_Uint:
        {
            return QString::fromUtf8("uint");
        }
        case PropertyType_Int:
        {
            return QString::fromUtf8("int");
        }
        case PropertyType_Float:
        {
            return QString::fromUtf8("float");
        }
        case PropertyType_Link:
        {
            return QString::fromUtf8("link");
        }
        case PropertyType_String:
        {
            return QString::fromUtf8("string");
        }
        case PropertyType_FilePath:
        {
            return QString::fromUtf8("filepath");
        }
        default:
        {
            return QString::fromUtf8("invalid");
        }
    }
}

Property::PropertyType Property::Type() const
{
    return mType;
}

Property Property::FromQString(PropertyType type, const QString& valueString)
{
    Property theProp;
    theProp.mType = type;

    switch(type)
    {
        //Add cases if you need to handle something weird
        default:
        {
            theProp.setValue(valueString);
            return theProp;
        }
    }
}

Property Property::FromQString(const QString& typeString, const QString& valueString)
{
    Property theProp;

    //If the size is 0 then it's an old prop without a type
    if(typeString.compare(QString::fromUtf8("string")) == 0)
    {
        theProp.setValue(valueString);
        theProp.mType = PropertyType_String;
    }
    else if(typeString.compare(QString::fromUtf8("filepath")) == 0)
    {
        theProp.setValue(valueString);
        theProp.mType = PropertyType_FilePath;
    }
    else if(typeString.compare(QString::fromUtf8("uint")) == 0)
    {
        theProp.setValue(valueString.toUInt());
        theProp.mType = PropertyType_Uint;
    }
    else if(typeString.compare(QString::fromUtf8("int")) == 0)
    {
        theProp.setValue(valueString.toInt());
        theProp.mType = PropertyType_Int;
    }
    else if(typeString.compare(QString::fromUtf8("float")) == 0)
    {
        theProp.setValue(valueString.toFloat());
        theProp.mType = PropertyType_Float;
    }
    else if(typeString.compare(QString::fromUtf8("link")) == 0)
    {
        theProp.setValue(valueString.toUInt());
        theProp.mType = PropertyType_Link;
    }
    //Default is a string
    else
    {
        theProp.setValue(valueString);
        theProp.mType = PropertyType_String;
    }

    return theProp;
}

QString Property::ToQString() const
{
    switch(mType)
    {
        //If it's anything else, just output the string
        default:
        {
            return toString();
        }
    }
}

void Properties::merge(const Properties &other)
{
    // Based on QMap::unite, but using insert instead of insertMulti
    const_iterator it = other.constEnd();
    const const_iterator b = other.constBegin();
    while (it != b) {
        --it;
        insert(it.key(), it.value());
    }
}
