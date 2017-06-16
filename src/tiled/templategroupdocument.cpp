/*
 * templategroupdocument.cpp
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

#include "templategroupdocument.h"

#include "templategroupformat.h"
#include "tmxmapformat.h"
#include "savefile.h"

#include <QCoreApplication>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

using namespace Tiled;
using namespace Tiled::Internal;

TemplateGroupDocument::TemplateGroupDocument(TemplateGroup *templateGroup, const QString &fileName)
    : Document(TemplateGroupDocumentType, fileName)
    , mTemplateGroup(templateGroup)
{
    mFileName = fileName;
}

TemplateGroupDocument::~TemplateGroupDocument()
{
}

bool TemplateGroupDocument::save(const QString &fileName, QString *error)
{
    auto format = TtxTemplateGroupFormat::instance();

    if (!format->write(mTemplateGroup, fileName)) {
        if (error)
            *error = format->errorString();
        return false;
    }

    return true;
}

TemplateGroupDocument *TemplateGroupDocument::load(const QString &fileName,
                                                   TemplateGroupFormat *format,
                                                   QString *error)
{
    TemplateGroup *templateGroup = format->read(fileName);

    if (!templateGroup) {
        if (error)
            *error = format->errorString();
        return nullptr;
    }

    templateGroup->setFormat(format);

    return new TemplateGroupDocument(templateGroup, fileName);
}

FileFormat *TemplateGroupDocument::writerFormat() const
{
    return mTemplateGroup->format();
}

QString TemplateGroupDocument::displayName() const
{
    QString displayName = mTemplateGroup->name();

    if (displayName.isEmpty())
        displayName = mFileName;

    return displayName;
}

void TemplateGroupDocument::addTemplate(ObjectTemplate *objectTemplate)
{
    mTemplateGroup->addTemplate(objectTemplate);
}


static void writeTemplateDocumentsXml(QFileDevice *device,
                                      const TemplateDocuments &templateDocuments)
{
    QXmlStreamWriter writer(device);

    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(1);

    writer.writeStartDocument();
    writer.writeStartElement(QLatin1String("templatedocuments"));

    for (const TemplateGroupDocument *templateDocument : templateDocuments) {
        writer.writeStartElement(QLatin1String("templatedocument"));

        writer.writeAttribute(QLatin1String("path"), templateDocument->fileName());

        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();
}

static void readTemplateDocumentsXml(QFileDevice *device,
                                     TemplateDocuments &templateDocuments,
                                     QString &error)
{
    QXmlStreamReader reader(device);

    if (!reader.readNextStartElement() || reader.name() != QLatin1String("templatedocuments")) {
        error = QCoreApplication::translate(
                    "TemplateDocuments", "File doesn't contain template documents.");
        return;
    }

    while (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("templatedocument")) {
            const QXmlStreamAttributes atts = reader.attributes();
            const QString path(atts.value(QLatin1String("path")).toString());

            auto templateGroupFormat = TtxTemplateGroupFormat::instance();

            // TODO: handle errors that might happen while loading
            auto templateGroupDocument = TemplateGroupDocument::load(path, templateGroupFormat);
            if (templateGroupDocument)
                templateDocuments.append(templateGroupDocument);
        }
        reader.skipCurrentElement();
    }

    if (reader.hasError()) {
        error = QCoreApplication::translate("TemplateDocuments",
                                             "%3\n\nLine %1, column %2")
                .arg(reader.lineNumber())
                .arg(reader.columnNumber())
                .arg(reader.errorString());
    }
}

static TemplateDocumentsSerializer::Format detectFormat(const QString &fileName)
{
    if (fileName.endsWith(QLatin1String(".json"), Qt::CaseInsensitive))
        return TemplateDocumentsSerializer::Json;
    else
        return TemplateDocumentsSerializer::Xml;
}

TemplateDocumentsSerializer::TemplateDocumentsSerializer(Format format)
    : mFormat(format)
{
}

bool TemplateDocumentsSerializer::writeTemplateDocuments(const QString &fileName,
                                                         const TemplateDocuments &templateDocuments)
{
    mError.clear();

    SaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate(
                    "TemplateDocuments", "Could not open file for writing.");
        return false;
    }

    Format format = mFormat;
    if (format == Autodetect)
        format = detectFormat(fileName);

    if (format == Xml)
        writeTemplateDocumentsXml(file.device(), templateDocuments);

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

bool TemplateDocumentsSerializer::readTemplateDocuments(const QString &fileName,
                                                        TemplateDocuments &templateDocuments)
{
    mError.clear();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate(
                    "TemplateDocuments", "Could not open file.");
        return false;
    }

    Format format = mFormat;
    if (format == Autodetect)
        format = detectFormat(fileName);

    if (format == Xml)
        readTemplateDocumentsXml(&file, templateDocuments, mError);

    return mError.isEmpty();
}
