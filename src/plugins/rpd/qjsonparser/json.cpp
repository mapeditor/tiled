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

#include <QTextCodec>
#include <qnumeric.h>

/*!
  \class JsonReader
  \reentrant

  \brief The JsonReader class provides a fast parser for reading
  well-formed JSON into a QVariant.

  The parser converts JSON types into QVariant types. For example, JSON
  arrays are translated into QVariantList and JSON objects are translated 
  into QVariantMap. For example,
  \code
  JsonReader reader;
  if (reader.parse(QString("{ \"id\": 123, \"class\": \"JsonReader\", \"authors\": [\"Denis\",\"Ettrich\",\"Girish\"] }"))) {
    QVariant result = reader.result();
    QVariantMap map = result.toMap(); // the JSON object
    qDebug() << map.count(); // 3
    qDebug() << map["id"].toInt(); // 123
    qDebug() << map["class"].toString(); // "JsonReader"
    QVariantList list = map["authors"].toList();
    qDebug() << list[2].toString(); // "Girish"
  } else {
    qDebug() << reader.errorString();
  }
  \endcode

  As seen above, the reader converts the JSON into a QVariant with arbitrary nesting.
  A complete listing of JSON to QVariant conversion is documented at parse().

  JsonWriter can be used to convert a QVariant into JSON string.
*/

/*!
  Constructs a JSON reader.
 */
JsonReader::JsonReader()
{
}

/*!
  Destructor
 */
JsonReader::~JsonReader()
{
}

/*!
  Parses the JSON \a ba as a QVariant.

  If the parse succeeds, this function returns true and the QVariant can
  be accessed using result(). If the parse fails, this function returns
  false and the error message can be accessed using errorMessage().

  The encoding of \ba is auto-detected based on the pattern of nulls in the
  initial 4 octets as described in "Section 3. Encoding" of RFC 2647. If an 
  encoding could not be auto-detected, this function assumes UTF-8.

  The conversion from JSON type into QVariant type is as listed below:
  \table
  \row
  \o false
  \o QVariant::Bool with the value false.
  \row
  \o true
  \o QVariant::Bool with the value true.
  \row
  \o null
  \o QVariant::Invalid i.e QVariant()
  \row
  \o object
  \o QVariant::Map i.e QVariantMap
  \row
  \o array
  \o QVariant::List i.e QVariantList
  \row
  \o string
  \o QVariant::String i.e QString
  \row
  \o number
  \o QVariant::Double or QVariant::LongLong. If the JSON number contains a '.' or 'e' 
     or 'E', QVariant::Double is used.
  \endtable

  The byte array \ba may or may not contain a BOM.
 */
bool JsonReader::parse(const QByteArray &ba)
{
    QTextCodec *codec = QTextCodec::codecForUtfText(ba, 0); // try BOM detection
    if (!codec) {
        int mib = 106; // utf-8

        if (ba.length() > 3) { // auto-detect
            const char *data = ba.constData();
            if (data[0] != 0) {
                if (data[1] != 0)
                    mib = 106; // utf-8
                else if (data[2] != 0)
                    mib = 1014; // utf16 le
                else
                    mib = 1019; // utf32 le
            } else if (data[1] != 0)
                mib = 1013; // utf16 be
            else
                mib = 1018; // utf32 be
        }
        codec = QTextCodec::codecForMib(mib);
    }
    QString str = codec->toUnicode(ba);
    return parse(str);
}

/*!
  Parses the JSON string \a str as a QVariant.

  If the parse succeeds, this function returns true and the QVariant can
  be accessed using result(). If the parse fails, this function returns
  false and the error message can be accessed using errorMessage().
 */
bool JsonReader::parse(const QString &str)
{
    JsonLexer lexer(str);
    JsonParser parser;
    if (!parser.parse(&lexer)) {
        m_errorString = parser.errorMessage();
        m_result = QVariant();
        return false;
    }
    m_errorString.clear();
    m_result = parser.result();
    return true;
}

/*!
  Returns the result of the last parse() call.

  If parse() failed, this function returns an invalid QVariant.
 */
QVariant JsonReader::result() const
{
    return m_result;
}

/*!
  Returns the error message for the last parse() call.

  If parse() succeeded, this functions return an empty string. The error message
  should be used for debugging purposes only.
 */
QString JsonReader::errorString() const
{
    return m_errorString;
}

/*!
  \class JsonWriter
  \reentrant

  \brief The JsonWriter class converts a QVariant into a JSON string.

  The writer converts specific supported types stored in a QVariant into JSON.
  For example,
  \code
    QVariantMap map;
    map["id"] = 123;
    map["class"] = "JsonWriter";
    QVariantList list;
    list << "Denis" << "Ettrich" << "Girish";
    map["authors"] = list;

    JsonWriter writer;
    if (writer.stringify(map)) {
        QString json = writer.result();
        qDebug() << json; // {"authors": ["Denis", "Ettrich", "Girish"], "class": "JsonWriter", "id": 123 }
    } else {
        qDebug() << "Failed to stringify " << writer.errorString();
    }
  \endcode

  The list of QVariant types that the writer supports is listed in stringify(). Note that
  custom C++ types registered using Q_DECLARE_METATYPE are not supported.
*/

/*!
  Creates a JsonWriter.
 */
JsonWriter::JsonWriter()
    : m_autoFormatting(false), m_autoFormattingIndent(4, QLatin1Char(' '))
{
}

/*!
  Destructor.
 */
JsonWriter::~JsonWriter()
{
}

/*!
  Enables auto formatting if \a enable is \c true, otherwise
  disables it.
 
  When auto formatting is enabled, the writer automatically inserts
  spaces and new lines to make the output more human readable.
 
  The default value is \c false.
 */
void JsonWriter::setAutoFormatting(bool enable)
{
    m_autoFormatting = enable;
}

/*!
  Returns \c true if auto formattting is enabled, otherwise \c false.
 */
bool JsonWriter::autoFormatting() const
{
    return m_autoFormatting;
}

/*!
  Sets the number of spaces or tabs used for indentation when
  auto-formatting is enabled. Positive numbers indicate spaces,
  negative numbers tabs.

  The default indentation is 4.

  \sa setAutoFormatting()
*/
void JsonWriter::setAutoFormattingIndent(int spacesOrTabs)
{
    m_autoFormattingIndent = QString(qAbs(spacesOrTabs), QLatin1Char(spacesOrTabs >= 0 ? ' ' : '\t'));
}

/*!
  Retuns the numbers of spaces or tabs used for indentation when
  auto-formatting is enabled. Positive numbers indicate spaces,
  negative numbers tabs.

  The default indentation is 4.

  \sa setAutoFormatting()
*/
int JsonWriter::autoFormattingIndent() const
{
    return m_autoFormattingIndent.count(QLatin1Char(' ')) - m_autoFormattingIndent.count(QLatin1Char('\t'));
}

/*! \internal
  Inserts escape character \ for characters in string as described in JSON specification.
 */
static QString escape(const QVariant &variant)
{
    QString str = variant.toString();
    QString res;
    res.reserve(str.length());
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == QLatin1Char('\b')) {
            res += QLatin1String("\\b");
        } else if (str[i] == QLatin1Char('\f')) {
            res += QLatin1String("\\f");
        } else if (str[i] == QLatin1Char('\n')) {
            res += QLatin1String("\\n");
        } else if (str[i] == QLatin1Char('\r')) {
            res += QLatin1String("\\r");
        } else if (str[i] == QLatin1Char('\t')) {
            res += QLatin1String("\\t");
        } else if (str[i] == QLatin1Char('\"')) {
            res += QLatin1String("\\\"");
        } else if (str[i] == QLatin1Char('\\')) {
            res += QLatin1String("\\\\");
        } else if (str[i] == QLatin1Char('/')) {
            res += QLatin1String("\\/");
        } else if (str[i].unicode() > 127) {
            res += QLatin1String("\\u") + QString::number(str[i].unicode(), 16).rightJustified(4, QLatin1Char('0'));
        } else {
            res += str[i];
        }
    }
    return res;
}

/*! \internal
  Stringifies \a variant.
 */
void JsonWriter::stringify(const QVariant &variant, int depth)
{
    if (variant.type() == QVariant::List || variant.type() == QVariant::StringList) {
        m_result += QLatin1Char('[');
        QVariantList list = variant.toList();
        for (int i = 0; i < list.count(); i++) {
            if (i != 0) {
                m_result += QLatin1Char(',');
                if (m_autoFormatting)
                    m_result += QLatin1Char(' ');
            }
            stringify(list[i], depth+1);
        }
        m_result += QLatin1Char(']');
    } else if (variant.type() == QVariant::Map) {
        QString indent = m_autoFormattingIndent.repeated(depth);
        QVariantMap map = variant.toMap();
        if (m_autoFormatting && depth != 0) {
            m_result += QLatin1Char('\n');
            m_result += indent;
            m_result += QLatin1String("{\n");
        } else {
            m_result += QLatin1Char('{');
        }
        for (QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it) {
            if (it != map.constBegin()) {
                m_result += QLatin1Char(',');
                if (m_autoFormatting)
                    m_result += QLatin1Char('\n');
            }
            if (m_autoFormatting)
                m_result += indent + QLatin1Char(' ');
            m_result += QLatin1Char('\"') + escape(it.key()) + QLatin1String("\":");
            stringify(it.value(), depth+1);
        }
        if (m_autoFormatting) {
            m_result += QLatin1Char('\n');
            m_result += indent;
        }
        m_result += QLatin1Char('}');
    } else if (variant.type() == QVariant::String || variant.type() == QVariant::ByteArray) {
        m_result += QLatin1Char('\"') + escape(variant) + QLatin1Char('\"');
    } else if (variant.type() == QVariant::Double || (int)variant.type() == (int)QMetaType::Float) {
        double d = variant.toDouble();
        if (qIsFinite(d))
            m_result += QString::number(variant.toDouble(), 'g', 15);
        else
            m_result += QLatin1String("null");
    } else if (variant.type() == QVariant::Bool) {
        m_result += variant.toBool() ? QLatin1String("true") : QLatin1String("false");
    } else if (variant.type() == QVariant::Invalid) {
        m_result += QLatin1String("null");
    } else if (variant.type() == QVariant::ULongLong) {
        m_result += QString::number(variant.toULongLong());
    } else if (variant.type() == QVariant::LongLong) {
        m_result += QString::number(variant.toLongLong());
    } else if (variant.type() == QVariant::Int) {
        m_result += QString::number(variant.toInt());
    } else if (variant.type() == QVariant::UInt) {
        m_result += QString::number(variant.toUInt());
    } else if (variant.type() == QVariant::Char) {
        QChar c = variant.toChar();
        if (c.unicode() > 127)
            m_result += QLatin1String("\"\\u") + QString::number(c.unicode(), 16).rightJustified(4, QLatin1Char('0')) + QLatin1Char('\"');
        else
            m_result += QLatin1Char('\"') + c + QLatin1Char('\"');
    } else if (variant.canConvert<qlonglong>()) {
        m_result += QString::number(variant.toLongLong());
    } else if (variant.canConvert<QString>()) {
        m_result += QLatin1Char('\"') + escape(variant) + QLatin1Char('\"');
    } else {
        if (!m_errorString.isEmpty())
            m_errorString.append(QLatin1Char('\n'));
        QString msg = QString::fromLatin1("Unsupported type %1 (id: %2)").arg(QString::fromUtf8(variant.typeName())).arg(variant.userType());
        m_errorString.append(msg);
        qWarning() << "JsonWriter::stringify - " << msg;
        m_result += QLatin1String("null");
    }
}

/*!
  Converts the variant \a var into a JSON string.

  The stringizer converts \a var into JSON based on the type of it's contents. The supported
  types and their conversion into JSON is as listed below:

  \table
  \row
  \o QVariant::List, QVariant::StringList
  \o JSON array []
  \row
  \o QVariant::Map
  \o JSON object {}
  \row
  \o QVariant::String, QVariant::ByteArray
  \o JSON string encapsulated in double quotes. String contents are escaped using '\' if necessary.
  \row
  \o QVariant::Double, QMetaType::Float
  \o JSON number with a precision 15. Infinity and NaN are converted into null.
  \row
  \o QVariant::Bool
  \o JSON boolean true and false
  \row
  \o QVariant::Invalid
  \o JSON null
  \row
  \o QVariant::ULongLong, QVariant::LongLong, QVariant::Int, QVariant::UInt, 
  \o JSON number
  \row
  \o QVariant::Char
  \o JSON string. Non-ASCII characters are converted into the \uXXXX notation.
  \endtable

  As a fallback, the writer attempts to convert a type not listed above into a long long or a
  QString using QVariant::canConvert. See the QVariant documentation for possible conversions.

  JsonWriter does not support stringizing custom user types stored in the QVariant. Any such
  value would be converted into JSON null.
 */
bool JsonWriter::stringify(const QVariant &var)
{
    m_errorString.clear();
    m_result.clear();
    stringify(var, 0 /* depth */);
    return m_errorString.isEmpty();
}

/*!
  Returns the result of the last stringify() call.

  If stringify() failed, this function returns a null QString.
 */
QString JsonWriter::result() const
{
    return m_result;
}

/*!
  Returns the error message for the last stringify() call.

  If stringify() succeeded, this functions return an empty string. The error message
  should be used for debugging purposes only.
 */
QString JsonWriter::errorString() const
{
    return m_errorString;
}

