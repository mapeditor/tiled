/*
 * mapeditor.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindijer.nl>
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

#include "mapeditor.h"

#include "brokenlinks.h"
#include "layerdock.h"
#include "mapscene.h"
#include "mapview.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>

namespace Tiled {
namespace Internal {

class FileChangedWarning : public QWidget
{
    Q_OBJECT

public:
    FileChangedWarning(QWidget *parent = nullptr)
        : QWidget(parent)
        , mLabel(new QLabel(this))
        , mButtons(new QDialogButtonBox(QDialogButtonBox::Yes |
                                        QDialogButtonBox::No,
                                        Qt::Horizontal,
                                        this))
    {
        mLabel->setText(tr("File change detected. Discard changes and reload the map?"));

        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(mLabel);
        layout->addStretch(1);
        layout->addWidget(mButtons);
        setLayout(layout);

        connect(mButtons, SIGNAL(accepted()), SIGNAL(reload()));
        connect(mButtons, SIGNAL(rejected()), SIGNAL(ignore()));
    }

signals:
    void reload();
    void ignore();

private:
    QLabel *mLabel;
    QDialogButtonBox *mButtons;
};

class MapViewContainer : public QWidget
{
    Q_OBJECT

public:
    MapViewContainer(MapView *mapView,
                     MapDocument *mapDocument,
                     QWidget *parent = nullptr)
        : QWidget(parent)
        , mMapView(mapView)
        , mWarning(new FileChangedWarning)
        , mBrokenLinksModel(new BrokenLinksModel(mapDocument, this))
        , mBrokenLinksWidget(nullptr)
    {
        mWarning->setVisible(false);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setMargin(0);
        layout->setSpacing(0);

        if (mBrokenLinksModel->hasBrokenLinks()) {
            mBrokenLinksWidget = new BrokenLinksWidget(mBrokenLinksModel, this);
            layout->addWidget(mBrokenLinksWidget);

            connect(mBrokenLinksWidget, &BrokenLinksWidget::ignore,
                    this, &MapViewContainer::deleteBrokenLinksWidget);
        }

        connect(mBrokenLinksModel, &BrokenLinksModel::hasBrokenLinksChanged,
                this, &MapViewContainer::hasBrokenLinksChanged);

        layout->addWidget(mapView);
        layout->addWidget(mWarning);

        connect(mWarning, &FileChangedWarning::reload, this, &MapViewContainer::reload);
        connect(mWarning, &FileChangedWarning::ignore, mWarning, &FileChangedWarning::hide);
    }

    MapView *mapView() const { return mMapView; }

    void setFileChangedWarningVisible(bool visible)
    { mWarning->setVisible(visible); }

signals:
    void reload();

private slots:
    void hasBrokenLinksChanged(bool hasBrokenLinks)
    {
        if (!hasBrokenLinks)
            deleteBrokenLinksWidget();
    }

    void deleteBrokenLinksWidget()
    {
        if (mBrokenLinksWidget) {
            mBrokenLinksWidget->deleteLater();
            mBrokenLinksWidget = nullptr;
        }
    }

private:
    MapView *mMapView;

    FileChangedWarning *mWarning;
    BrokenLinksModel *mBrokenLinksModel;
    BrokenLinksWidget *mBrokenLinksWidget;
};



MapEditor::MapEditor(QWidget *parent)
    : QMainWindow(parent)
    , mLayerDock(new LayerDock(this))
    , mWidgetStack(new QStackedWidget(this))
    , mSelectedTool(nullptr)
    , mViewWithTool(nullptr)
{
    setWindowFlags(windowFlags() & ~Qt::Window);

    addDockWidget(Qt::RightDockWidgetArea, mLayerDock);

    setCentralWidget(mWidgetStack);

    //    mLayerDock->setMapDocument(mapDocument);
}

void MapEditor::addMapDocument(MapDocument *mapDocument)
{
    MapView *view = new MapView;
    MapScene *scene = new MapScene(view); // scene is owned by the view
    MapViewContainer *container = new MapViewContainer(view, mapDocument, mWidgetStack);

    scene->setMapDocument(mapDocument);
    view->setScene(scene);

    mWidgetStack->addWidget(container);
    mWidgetForMap.insert(mapDocument, container);
}

void MapEditor::removeMapDocument(MapDocument *mapDocument)
{
    delete mWidgetForMap.take(mapDocument);
}

void MapEditor::setCurrentMapDocument(MapDocument *mapDocument)
{
    MapViewContainer *container = mWidgetForMap.value(mapDocument);
    mWidgetStack->setCurrentWidget(container);

    /*
    if (mViewWithTool) {
        MapScene *mapScene = mViewWithTool->mapScene();
        mapScene->disableSelectedTool();
        mViewWithTool = nullptr;
    }

    if (MapView *mapView = currentMapView()) {
        MapScene *mapScene = mapView->mapScene();
        mapScene->setSelectedTool(mSelectedTool);
        mapScene->enableSelectedTool();
        if (mSelectedTool)
            mapView->viewport()->setCursor(mSelectedTool->cursor());
        else
            mapView->viewport()->unsetCursor();
        mViewWithTool = mapView;
    }
    */
}

} // namespace Internal
} // namespace Tiled

#include "mapeditor.moc"
