/*
 * rtbtileselectionmanager.h
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

#ifndef RTBTILESELECTIONMANAGER_H
#define RTBTILESELECTIONMANAGER_H

#include <QObject>

class QAction;
class QActionGroup;

namespace Tiled {
namespace Internal {

class RTBTileButton;
class MapDocument;
class AbstractTool;
class StampBrush;
class BucketFillTool;

class RTBTileSelectionManager : public QObject
{
    Q_OBJECT

public:
    RTBTileSelectionManager(QObject *parent = 0);
    ~RTBTileSelectionManager();

    void setMapDocument(MapDocument *mapDocument);

    QAction *registerTile(RTBTileButton *tileButton);
    void selectTileButton(RTBTileButton *tileButton);

    void setStampBrush(StampBrush *stampBrush);

    void setBucketFillTool(BucketFillTool *bucketFillTool);

    void setSeparatorAction(QAction *separatorAction);

signals:
    void selectedTileChanged(AbstractTool *tileButton);

    /**
     * Emitted when the status information of the current tool changed.
     * @see AbstractTool::setStatusInfo()
     */
    void statusInfoChanged(const QString &info);

private slots:
    void actionTriggered(QAction *action);
    void tileButtonEnabledChanged(bool enabled);
    void tileButtonVisibleChanged(bool visible);
    void selectEnabledTileButton();
    void toggleSeparator(bool visible);
    void createStamp(RTBTileButton *tileButton);
    void createStamp(int type);
    void updateIcon(QAction *action, RTBTileButton *tileButton);

private:
    Q_DISABLE_COPY(RTBTileSelectionManager)

    void setSelectedTileButton(RTBTileButton *tileButton);

    QActionGroup *mActionGroup;
    RTBTileButton *mSelectedTileButton;
    RTBTileButton *mPreviouslyDisabledTileButton;
    MapDocument *mMapDocument;

    StampBrush *mStampBrush;
    BucketFillTool *mBucketFillTool;

    QAction *mSeparatorAction;

};

} // namespace Internal
} // namespace Tiled

#endif // RTBTILESELECTIONMANAGER_H
