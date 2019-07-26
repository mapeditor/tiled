/*
 * scriptedmapformat.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include "mapformat.h"

#include <QJSValue>

namespace Tiled {

class ScriptedMapFormat final : public MapFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapFormat)

public:
    ScriptedMapFormat(const QString &shortName, const QJSValue &object,
                      QObject *parent = nullptr);
    ~ScriptedMapFormat() override;

    // FileFormat interface
    Capabilities capabilities() const override;
    QString nameFilter() const override;
    QString shortName() const override;
    bool supportsFile(const QString &fileName) const override;
    QString errorString() const override;

    // MapFormat interface
#if 0
    QStringList outputFiles(const Map *map, const QString &fileName) const override;
#endif
    std::unique_ptr<Map> read(const QString &fileName) override;
    bool write(const Map *map, const QString &fileName, Options options) override;

    static bool validateMapFormatObject(const QJSValue &value);

private:
    QString mShortName;
    QString mError;
    QJSValue mObject;
};


inline QString ScriptedMapFormat::shortName() const
{
    return mShortName;
}

inline QString ScriptedMapFormat::errorString() const
{
    return mError;
}

} // namespace Tiled
