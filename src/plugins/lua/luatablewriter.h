/*
 * Lua Tiled Plugin
 * Copyright 2011-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include <QString>
#include <QVariant>

class QIODevice;

namespace Lua {

/**
 * Makes it easy to produce a well formatted Lua table.
 */
class LuaTableWriter
{
public:
    LuaTableWriter(QIODevice *device);

    void writeStartDocument();
    void writeEndDocument();

    void writeStartTable();
    void writeStartReturnTable();
    void writeStartTable(const QByteArray &name);
    void writeEndTable();

    void writeValue(int value);
    void writeValue(unsigned value);
    void writeValue(const QByteArray &value);
    void writeValue(const QString &value);

    void writeUnquotedValue(const QByteArray &value);

    void writeKeyAndValue(const QByteArray &key, int value);
    void writeKeyAndValue(const QByteArray &key, unsigned value);
    void writeKeyAndValue(const QByteArray &key, float value);
    void writeKeyAndValue(const QByteArray &key, double value);
    void writeKeyAndValue(const QByteArray &key, bool value);
    void writeKeyAndValue(const QByteArray &key, const char *value);
    void writeKeyAndValue(const QByteArray &key, const QByteArray &value);
    void writeKeyAndValue(const QByteArray &key, const QString &value);

    void writeQuotedKeyAndValue(const QString &key, const QVariant &value);
    void writeKeyAndUnquotedValue(const QByteArray &key,
                                  const QByteArray &value);

    void setSuppressNewlines(bool suppressNewlines);
    bool suppressNewlines() const;

    void setMinimize(bool minimize);
    bool minimize() const;

    void prepareNewLine();

    bool hasError() const { return m_error; }

    static QString quote(const QString &str);

private:
    void prepareNewValue();
    void writeIndent();

    void writeNewline();
    void write(const char *bytes, qint64 length);
    void write(const char *bytes);
    void write(const QByteArray &bytes);
    void write(char c);

    QIODevice *m_device;
    int m_indent { 0 };
    char m_valueSeparator { ',' };
    bool m_suppressNewlines { false };
    bool m_minimize { false };
    bool m_newLine { true };
    bool m_valueWritten { false };
    bool m_error { false };
};

inline void LuaTableWriter::writeValue(int value)
{ writeUnquotedValue(QByteArray::number(value)); }

inline void LuaTableWriter::writeValue(unsigned value)
{ writeUnquotedValue(QByteArray::number(value)); }

inline void LuaTableWriter::writeValue(const QString &value)
{ writeUnquotedValue(quote(value).toUtf8()); }

inline void LuaTableWriter::writeKeyAndValue(const QByteArray &key, int value)
{ writeKeyAndUnquotedValue(key, QByteArray::number(value)); }

inline void LuaTableWriter::writeKeyAndValue(const QByteArray &key, unsigned value)
{ writeKeyAndUnquotedValue(key, QByteArray::number(value)); }

inline void LuaTableWriter::writeKeyAndValue(const QByteArray &key, float value)
{ writeKeyAndValue(key, static_cast<double>(value)); }

inline void LuaTableWriter::writeKeyAndValue(const QByteArray &key, double value)
{ writeKeyAndUnquotedValue(key, QByteArray::number(value)); }

inline void LuaTableWriter::writeKeyAndValue(const QByteArray &key, bool value)
{ writeKeyAndUnquotedValue(key, value ? "true" : "false"); }

inline void LuaTableWriter::writeKeyAndValue(const QByteArray &key, const QString &value)
{ writeKeyAndUnquotedValue(key, quote(value).toUtf8()); }

inline void LuaTableWriter::write(const char *bytes)
{ write(bytes, qstrlen(bytes)); }

inline void LuaTableWriter::write(const QByteArray &bytes)
{ write(bytes.constData(), bytes.length()); }

inline void LuaTableWriter::write(char c)
{ write(&c, 1); }

/**
 * Sets whether newlines should be suppressed. While newlines are suppressed,
 * the writer will write out spaces instead of newlines.
 */
inline void LuaTableWriter::setSuppressNewlines(bool suppressNewlines)
{ m_suppressNewlines = suppressNewlines; }

inline bool LuaTableWriter::suppressNewlines() const
{ return m_suppressNewlines; }

/**
 * Sets whether output should be minimized. This implies suppressing newlines
 * and in addition will avoid printing unnecessary spaces.
 */
inline void LuaTableWriter::setMinimize(bool minimize)
{ m_minimize = minimize; }

inline bool LuaTableWriter::minimize() const
{ return m_minimize; }

} // namespace Lua
