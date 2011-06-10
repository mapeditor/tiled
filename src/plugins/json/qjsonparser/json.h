/****************************************************************************
**
** Copyright (c) 2010 Girish Ramakrishnan <girish@forwardbias.in>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#ifndef JSON_H
#define JSON_H

#include <QByteArray>
#include <QVariant>

namespace Json {
    QVariant parse(const QByteArray &utf8, QString *error = 0);
    QByteArray stringify(const QVariant &variant, int depth = 0); // UTF-8 encoded
};

#endif // JSON_H

