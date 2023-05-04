/*
 * jsonwriter.h
 * Copyright 2011-2021, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#pragma once

#include <QByteArray>
#include <QStack>
#include <QString>
#include <QVariant>

class QIODevice;

namespace Yy {

/**
 * Writes a JSON file, providing a high level of control over its contents.
 *
 * Using this writer, as opposed to the QJsonDocument, gives control over the
 * order of the properties and when to use newlines.
 */
class JsonWriter
{
    enum Scope {
        Array,
        Object
    };

public:
    JsonWriter(QIODevice *device);

    void writeEndDocument();

    void writeStartObject()                     { writeStartScope(Object); }
    void writeStartObject(const char *name)     { writeStartScope(Object, name); }
    void writeEndObject()                       { writeEndScope(Object); }

    void writeStartArray()                      { writeStartScope(Array); }
    void writeStartArray(const char *name)      { writeStartScope(Array, name); }
    void writeEndArray()                        { writeEndScope(Array); }

    void writeValue(int value);
    void writeValue(unsigned value);
    void writeValue(double value);
    void writeValue(const QByteArray &value);
    void writeValue(const QString &value);
    void writeValue(const QJsonValue &value);

    void writeUnquotedValue(const QByteArray &value);

    void writeMember(const char *key, int value);
    void writeMember(const char *key, unsigned value);
    void writeMember(const char *key, float value);
    void writeMember(const char *key, double value);
    void writeMember(const char *key, bool value);
    void writeMember(const char *key, const char *value);
    void writeMember(const char *key, const QByteArray &value);
    void writeMember(const char *key, const QString &value);
    void writeMember(const char *key, const QJsonValue &value);

    void writeUnquotedMember(const char *key,
                             const QByteArray &value);

    void setSuppressNewlines(bool suppressNewlines);
    bool suppressNewlines() const;

    void setMinimize(bool minimize);
    bool minimize() const;

    void prepareNewLine();

    bool hasError() const { return m_error; }

    static QString quote(const QString &str);

private:
    void writeStartScope(Scope scope);
    void writeStartScope(Scope scope, const char *name);
    void writeEndScope(Scope scope);

    void prepareNewValue();
    void writeIndent();

    void writeNewline();
    void writeKey(const char *key);
    void write(const char *bytes, qint64 length);
    void write(const char *bytes);
    void write(const QByteArray &bytes);
    void write(char c);

    QIODevice *m_device;

    QStack<Scope> m_scopes;
    char m_valueSeparator { ',' };
    bool m_suppressNewlines { false };
    bool m_minimize { false };
    bool m_newLine { true };
    bool m_valueWritten { false };
    bool m_error { false };
};

inline void JsonWriter::writeValue(int value)
{ writeUnquotedValue(QByteArray::number(value)); }

inline void JsonWriter::writeValue(unsigned value)
{ writeUnquotedValue(QByteArray::number(value)); }

inline void JsonWriter::writeValue(const QString &value)
{ writeUnquotedValue(quote(value).toUtf8()); }

inline void JsonWriter::writeMember(const char *key, int value)
{ writeUnquotedMember(key, QByteArray::number(value)); }

inline void JsonWriter::writeMember(const char *key, unsigned value)
{ writeUnquotedMember(key, QByteArray::number(value)); }

inline void JsonWriter::writeMember(const char *key, float value)
{ writeMember(key, static_cast<double>(value)); }

inline void JsonWriter::writeMember(const char *key, double value)
{ writeKey(key); writeValue(value); }

inline void JsonWriter::writeMember(const char *key, bool value)
{ writeUnquotedMember(key, value ? "true" : "false"); }

inline void JsonWriter::writeMember(const char *key, const QString &value)
{ writeUnquotedMember(key, quote(value).toUtf8()); }

inline void JsonWriter::write(const char *bytes)
{ write(bytes, qstrlen(bytes)); }

inline void JsonWriter::write(const QByteArray &bytes)
{ write(bytes.constData(), bytes.length()); }

inline void JsonWriter::write(char c)
{ write(&c, 1); }

/**
 * Sets whether newlines should be suppressed.
 */
inline void JsonWriter::setSuppressNewlines(bool suppressNewlines)
{ m_suppressNewlines = suppressNewlines; }

inline bool JsonWriter::suppressNewlines() const
{ return m_suppressNewlines; }

/**
 * Sets whether output should be minimized. This implies suppressing newlines
 * and in addition will avoid printing unnecessary spaces.
 */
inline void JsonWriter::setMinimize(bool minimize)
{ m_minimize = minimize; }

inline bool JsonWriter::minimize() const
{ return m_minimize; }

} // namespace Yy
