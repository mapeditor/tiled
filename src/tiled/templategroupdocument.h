#pragma once

#include "document.h"
#include "templategroup.h"

namespace Tiled {

class TemplateGroupFormat;

namespace Internal {

class MapDocument;

class TemplateGroupDocument : public Document
{
    Q_OBJECT

public:
    TemplateGroupDocument(TemplateGroup *templateGroup, const QString &fileName = QString());
    ~TemplateGroupDocument();

    bool save(const QString &fileName, QString *error = nullptr) override;

    static TemplateGroupDocument *load(const QString &fileName,
                                       TemplateGroupFormat *format,
                                       QString *error = nullptr);

    FileFormat *writerFormat() const override;
    QString displayName() const override;

private:
    TemplateGroup *mTemplateGroup;
};

} // namespace Internal
} // namespace Tiled
