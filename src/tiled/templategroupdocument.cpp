#include "templategroupdocument.h"

#include "templategroupformat.h"

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
    TemplateGroupFormat *templateGroupFormat = mTemplateGroup->format();

    if (!templateGroupFormat->write(mTemplateGroup, fileName)) {
        if (error)
            *error = templateGroupFormat->errorString();
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

    if(displayName.isEmpty())
        displayName = mFileName;

    return displayName;
}

void TemplateGroupDocument::addTemplate(ObjectTemplate *objectTemplate)
{
    mTemplateGroup->addTemplate(objectTemplate);
}
