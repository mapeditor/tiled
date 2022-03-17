/****************************************************************************
**
** Copyright (c) 2010 Girish Ramakrishnan <girish@forwardbias.in>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#pragma once

#include <QByteArray>
#include <QVariant>

class JsonWriter
{
public:
    JsonWriter();
    ~JsonWriter();

    bool stringify(const QVariant &variant);

    QString result() const;

    QString errorString() const;

    void setAutoFormatting(bool autoFormat);
    bool autoFormatting() const;

    void setAutoFormattingIndent(int spaceOrTabs);
    int autoFormattingIndent() const;

    void setAutoFormattingWrapArrayCount(int count);

private:
    void stringify(const QVariant &variant, int depth);

    QString m_result;
    QString m_errorString;
    bool m_autoFormatting = false;
    QString m_autoFormattingIndent;
    int m_autoFormattingWrapArrayCount = 0;
};
