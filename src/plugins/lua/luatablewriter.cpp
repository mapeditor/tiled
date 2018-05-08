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

#include "luatablewriter.h"

#include <QIODevice>

namespace Lua {

LuaTableWriter::LuaTableWriter(QIODevice *device)
    : m_device(device)
    , m_indent(0)
    , m_valueSeparator(',')
    , m_suppressNewlines(false)
    , m_newLine(true)
    , m_valueWritten(false)
    , m_error(false)
{
}

void LuaTableWriter::writeStartDocument()
{
    Q_ASSERT(m_indent == 0);
}

void LuaTableWriter::writeEndDocument()
{
    Q_ASSERT(m_indent == 0);
    write('\n');
}

void LuaTableWriter::writeStartTable()
{
    prepareNewLine();
    write('{');
    ++m_indent;
    m_newLine = false;
    m_valueWritten = false;
}

void LuaTableWriter::writeStartReturnTable()
{
    prepareNewLine();
    write("return {");
    ++m_indent;
    m_newLine = false;
    m_valueWritten = false;
}

void LuaTableWriter::writeStartTable(const QByteArray &name)
{
    prepareNewLine();
    write(name + " = {");
    ++m_indent;
    m_newLine = false;
    m_valueWritten = false;
}

void LuaTableWriter::writeEndTable()
{
    Q_ASSERT(m_indent > 0);
    --m_indent;
    if (m_valueWritten)
        writeNewline();
    write('}');
    m_newLine = false;
    m_valueWritten = true;
}

void LuaTableWriter::writeValue(const QByteArray &value)
{
    prepareNewValue();
    write('"');
    write(value);
    write('"');
    m_newLine = false;
    m_valueWritten = true;
}

void LuaTableWriter::writeUnquotedValue(const QByteArray &value)
{
    prepareNewValue();
    write(value);
    m_newLine = false;
    m_valueWritten = true;
}

void LuaTableWriter::writeKeyAndValue(const QByteArray &key,
                                      const char *value)
{
    prepareNewLine();
    write(key);
    write(" = \"");
    write(value);
    write('"');
    m_newLine = false;
    m_valueWritten = true;
}

void LuaTableWriter::writeKeyAndValue(const QByteArray &key,
                                      const QByteArray &value)
{
    prepareNewLine();
    write(key);
    write(" = \"");
    write(value);
    write('"');
    m_newLine = false;
    m_valueWritten = true;
}

void LuaTableWriter::writeQuotedKeyAndValue(const QString &key,
                                            const QVariant &value)
{
    prepareNewLine();
    write('[');
    write(quote(key).toUtf8());
    write("] = ");

    switch (value.type()) {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
    case QVariant::Bool:
        write(value.toString().toLatin1());
        break;
    default:
        write(quote(value.toString()).toUtf8());
        break;
    }

    m_newLine = false;
    m_valueWritten = true;
}

void LuaTableWriter::writeKeyAndUnquotedValue(const QByteArray &key,
                                              const QByteArray &value)
{
    prepareNewLine();
    write(key);
    write(" = ");
    write(value);
    m_newLine = false;
    m_valueWritten = true;
}

/**
 * Quotes the given string, escaping special characters as necessary.
 */
QString LuaTableWriter::quote(const QString &str)
{
    QString quoted;
    quoted.reserve(str.length() + 2);   // most likely scenario
    quoted.append(QLatin1Char('"'));

    for (const QChar c : str) {
        switch (c.unicode()) {
        case '\\':  quoted.append(QLatin1String("\\\\"));  break;
        case '"':   quoted.append(QLatin1String("\\\""));  break;
        case '\n':  quoted.append(QLatin1String("\\n"));   break;
        default:    quoted.append(c);
        }
    }

    quoted.append(QLatin1Char('"'));
    return quoted;
}

void LuaTableWriter::prepareNewLine()
{
    if (m_valueWritten) {
        write(m_valueSeparator);
        m_valueWritten = false;
    }
    writeNewline();
}

void LuaTableWriter::prepareNewValue()
{
    if (!m_valueWritten) {
        writeNewline();
    } else {
        write(m_valueSeparator);
        write(' ');
    }
}

void LuaTableWriter::writeIndent()
{
    for (int level = m_indent; level; --level)
        write("  ");
}

void LuaTableWriter::writeNewline()
{
    if (!m_newLine) {
        if (m_suppressNewlines) {
            write(' ');
        } else {
            write('\n');
            writeIndent();
        }
        m_newLine = true;
    }
}

void LuaTableWriter::write(const char *bytes, qint64 length)
{
    if (m_device->write(bytes, length) != length)
        m_error = true;
}

} // namespace Lua
