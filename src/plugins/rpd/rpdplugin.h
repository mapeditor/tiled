#ifndef RPDPLUGIN_H
#define RPDPLUGIN_H

#include "rpd_global.h"

#include "layer.h"
#include "mapformat.h"
#include "plugin.h"
#include "tilesetformat.h"

#include <QObject>

namespace Tiled {
class Map;
}

namespace Rpd {

class RPDSHARED_EXPORT RpdPlugin : public Tiled::Plugin
{
    Q_OBJECT
    Q_INTERFACES(Tiled::Plugin)
    Q_PLUGIN_METADATA(IID "org.mapeditor.Plugin" FILE "plugin.json")

public:
    void initialize() override;
};


class RPDSHARED_EXPORT RpdMapFormat : public Tiled::WritableMapFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapFormat)

public:
    enum SubFormat {
        Rpd
    };

    enum TileId {
        ENTRANCE = 7,
        EXIT = 8,
        LOCKED_EXIT = 25,
        UNLOCKED_EXIT = 26
    };

    explicit RpdMapFormat(SubFormat subFormat, QObject *parent = nullptr);


    bool write(const Tiled::Map *map, const QString &fileName, Options options = Options()) override;
    QString shortName(void) const override;
    QString nameFilter() const override;
    QString errorString() const override;

protected:
    QString mError;
    SubFormat mSubFormat;
private:
    bool insertTilesetFile(Tiled::Layer &layer, const QString &tiles_name, QJsonObject &mapJson);

};


class RPDSHARED_EXPORT RpdTilesetFormat : public Tiled::TilesetFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::TilesetFormat)

public:
    explicit RpdTilesetFormat(QObject *parent = nullptr);

    Tiled::SharedTileset read(const QString &fileName) override;
    bool supportsFile(const QString &fileName) const override;

    bool write(const Tiled::Tileset &tileset, const QString &fileName, Options options = Options()) override;

    QString shortName(void) const override;
    QString nameFilter() const override;
    QString errorString() const override;

protected:
    QString mError;
};

} // namespace Rpd

#endif // RPDPLUGIN_H
