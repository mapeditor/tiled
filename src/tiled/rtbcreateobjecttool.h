/*
 * rtbcreateobjecttool.h
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#ifndef RTBCREATEOBJECTTOOL_H
#define RTBCREATEOBJECTTOOL_H

#include "CreateTileObjectTool.h"

namespace Tiled {

namespace Internal {


class RTBCreateObjectTool : public CreateTileObjectTool
{
    Q_OBJECT

public:
    RTBCreateObjectTool(QObject *parent, int type, int layerType);
    void languageChanged();
    void activate(MapScene *scene);

    QIcon errorIcon() { return mErrorIcon; }
    void setErrorIcon(QIcon errorIcon) { mErrorIcon = errorIcon; }

protected:
    void updateEnabledState();
    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument);

private:
    int mType;
    int mLayerType;
    QIcon mErrorIcon;

};

} // namespace Internal
} // namespace Tiled

#endif // RTBCREATEOBJECTTOOL_H
