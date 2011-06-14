/****************************************************************************
**
** Copyright (c) 2010 Girish Ramakrishnan <girish@forwardbias.in>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#include "json.h"
#include "jsonparser.cpp"

namespace Json {

QVariant parse(const QByteArray &json, QString *error)
{
    JsonLexer lexer(json);
    JsonParser parser;
    if (!parser.parse(&lexer) && error) {
        *error = parser.errorMessage();
    }
    return parser.result();
}

static QByteArray escape(const QVariant &variant)
{
    QString str = variant.toString();
    QByteArray res;
    res.reserve(str.length());
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == '\b') {
            res += "\\b";
        } else if (str[i] == '\f') {
            res += "\\f";
        } else if (str[i] == '\n') {
            res += "\\n";
        } else if (str[i] == '\r') {
            res += "\\r";
        } else if (str[i] == '\t') {
            res += "\\t";
        } else if (str[i] == '\"') {
            res += "\\\"";
        } else if (str[i] == '\\') {
            res += "\\\\";
        } else if (str[i].unicode() > 127) {
            res += "\\u" + QString::number(str[i].unicode(), 16).rightJustified(4, '0');
        } else {
            res += str[i].toAscii();
        }
    }
    return res;
}

QByteArray stringify(const QVariant &variant, int depth)
{
    QByteArray result;
    if (variant.type() == QVariant::List || variant.type() == QVariant::StringList) {
        result += "[";
        QVariantList list = variant.toList();
        for (int i = 0; i < list.count(); i++) {
            if (i != 0)
                result += ",";
            result += stringify(list[i]);
        }
        result += "]";
    } else if (variant.type() == QVariant::Map) {
        QByteArray indent(depth, ' ');
        QVariantMap map = variant.toMap();
        QVariantMap::const_iterator it = map.constBegin();
        result += "\n";
        result += indent;
        result += "{\n";
        while (it != map.constEnd()) {
            if (it != map.constBegin())
                result += ",\n";
            result += indent;
            result += " \"" + escape(it.key()) + "\":";
            result += stringify(it.value(), depth+1);
            ++it;
        }
        result += "\n";
        result += indent;
        result += "}";
    } else if (variant.type() == QVariant::String || variant.type() == QVariant::ByteArray) {
        result = "\"" + escape(variant) + "\"";
    } else if (variant.type() == QVariant::Double || (int)variant.type() == (int)QMetaType::Float) {
        result.setNum(variant.toDouble(), 'g', 15);
    } else if (variant.type() == QVariant::Bool) {
        result = variant.toBool() ? "true" : "false";
    } else if (variant.type() == QVariant::Invalid) {
        result = "null";
    } else if (variant.type() == QVariant::ULongLong) {
        result = QByteArray::number(variant.toULongLong());
    } else if (variant.type() == QVariant::LongLong) {
        result = QByteArray::number(variant.toLongLong());
    } else if (variant.type() == QVariant::Int) {
        result = QByteArray::number(variant.toInt());
    } else if (variant.type() == QVariant::UInt) {
        result = QByteArray::number(variant.toUInt());
    } else if (variant.type() == QVariant::Char) {
        QChar c = variant.toChar();
        if (c.unicode() > 127)
            result = "\\u" + QByteArray::number(c.unicode(), 16).rightJustified(4, '0');
        else
            result.append(c.toLatin1());
    } else if (variant.canConvert<qlonglong>()) {
        result = QByteArray::number(variant.toLongLong());
    } else if (variant.canConvert<QString>()) {
        result = "\"" + escape(variant) + "\"";
    }

    return result;
}

}

