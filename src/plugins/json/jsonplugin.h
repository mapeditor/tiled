#ifndef JSONPLUGIN_H
#define JSONPLUGIN_H

#include "json_global.h"

#include "mapwriterinterface.h"
#include "mapreaderinterface.h"
#include "map.h"


#include <QObject>
#include <QVariant>
#include <QDir>

using namespace Tiled;


Map *variantToMap(QVariantMap map, QDir mapDir=QDir());
QVariantMap mapToVariant(const Map *map, QDir mapDir=QDir());

class JSONSHARED_EXPORT JsonPlugin  : public QObject,
        public MapWriterInterface,
        public MapReaderInterface{
public:
    Q_OBJECT
    Q_INTERFACES(Tiled::MapWriterInterface Tiled::MapReaderInterface)

public:
    JsonPlugin();
    virtual ~JsonPlugin() {}

    Map *read(const QString &fileName);
    bool supportsFile(const QString &fileName) const;
    // MapWriterInterface
    bool write(const Map *map, const QString &fileName);
    QString nameFilter() const;
    QString errorString() const;

private:
    QString mError;
};

#endif // JSONPLUGIN_H
