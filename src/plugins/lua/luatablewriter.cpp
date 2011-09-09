/*
 * Lua Tiled Plugin
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
                                            const QString &value)
{
    prepareNewLine();
    write("[\"");
    write(key.toUtf8());
    write("\"] = \"");
    write(value.toUtf8());
    write('"');
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

void LuaTableWriter::write(const char *bytes, uint length)
{
    if (m_device->write(bytes, length) != length)
        m_error = true;
}

} // namespace Lua
