/*
 * rtbvalidatordock.cpp
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

#include "rtbvalidatordock.h"

#include "mapdocument.h"
#include "map.h"
#include "objectgroup.h"
#include "rtbvalidatorrule.h"
#include "rtbmapsettings.h"

#include <QHBoxLayout>
#include <qheaderview.h>

using namespace Tiled;
using namespace Tiled::Internal;

RTBValidatorDock::RTBValidatorDock(QWidget *parent)
    : QDockWidget(parent)
    , mValidatorView(new ValidatorView)
{
    setObjectName(QLatin1String("ValidatorDock"));

    QWidget *w = new QWidget(this);

    QVBoxLayout *vertical = new QVBoxLayout(w);
    vertical->setSpacing(0);
    vertical->setMargin(2);
    vertical->addWidget(mValidatorView);

    setWidget(w);
    retranslateUi();

    connect(mValidatorView, SIGNAL(validatorItemClicked(MapObject*))
                                   , this, SLOT(selectMapObject(MapObject*)));
    connect(mValidatorView, SIGNAL(validatorItemClicked(int))
                                   , this, SLOT(activateToolbarAction(int)));
}

RTBValidatorDock::~RTBValidatorDock()
{
    delete mValidatorView;
}

void RTBValidatorDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    mMapDocument = mapDocument;

    mValidatorView->setMapDocument(mapDocument);
}

void RTBValidatorDock::retranslateUi()
{
    setWindowTitle(tr("Validator"));
}

void RTBValidatorDock::selectMapObject(MapObject *mapObject)
{
    emit validatorItemClicked(mapObject);
}

void RTBValidatorDock::activateToolbarAction(int id)
{
    emit validatorItemClicked(id);
}

//=============================================================================

ValidatorView::ValidatorView(QWidget *parent):
    QTreeView(parent),
    mMapDocument(0)
{
    setRootIsDecorated(false);
    setHeaderHidden(true);
    setItemsExpandable(false);
    setUniformRowHeights(true);

    connect(this, SIGNAL(pressed(QModelIndex)),
            SLOT(indexPressed(QModelIndex)));
}

void ValidatorView::changeMinSize(int lenght)
{
    // new size: start size + (message lenght * half pixel size of the font)
    header()->setMinimumSectionSize(10 + (lenght * (fontInfo().pixelSize() / 2)));
}

QSize ValidatorView::sizeHint() const
{
    return QSize(320, 50);
}

void ValidatorView::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument)
    {
        mMapDocument->disconnect(this);
        disconnect(mMapDocument->validatorModel(), SIGNAL(maxLenghtChanged(int)),
                this, SLOT(changeMinSize(int)));
    }

    mMapDocument = mapDocument;

    if (mMapDocument)
    {
        RTBValidatorModel *validatorModel = mMapDocument->validatorModel();

        setModel(validatorModel);
        changeMinSize(validatorModel->maxMessageLenght());

        connect(validatorModel, SIGNAL(maxLenghtChanged(int)),
                this, SLOT(changeMinSize(int)));

        // update highlight if the mapdocument changed, e.g. toolbar actions
        mMapDocument->validatorModel()->highlightRules();
    }
    else
    {
        setModel(0);
    }
}

void ValidatorView::indexPressed(const QModelIndex &index)
{
    MapObject *mapObject = static_cast<RTBValidatorModel*>(model())->findMapObject(index.row());

    QList<MapObject *> objects = mMapDocument->map()->objectGroups().at(0)->objects();
    objects.append(mMapDocument->map()->objectGroups().at(1)->objects());

    // check if the rule contains an object and if the map still contains this object
    if(mapObject && objects.contains(mapObject))
        emit validatorItemClicked(mapObject);
    else
    {
        RTBValidatorRule * rule = static_cast<RTBValidatorModel*>(model())->findRule(index.row());
        if(rule->ruleID() == RTBValidatorRule::StartLocation)
        {
            if(mMapDocument->currentLayerIndex() != RTBMapSettings::ObjectID)
                mMapDocument->setCurrentLayerIndex(RTBMapSettings::ObjectID);

            emit validatorItemClicked(RTBMapObject::StartLocation);
        }
        else if(rule->ruleID() == RTBValidatorRule::FinishHole)
        {
            if(mMapDocument->currentLayerIndex() != RTBMapSettings::ObjectID)
                mMapDocument->setCurrentLayerIndex(RTBMapSettings::ObjectID);

            emit validatorItemClicked(RTBMapObject::FinishHole);
        }
        // map errors
        else if(rule->ruleID() == RTBValidatorRule::DefaultValues
                || rule->ruleID() == RTBValidatorRule::WallsAllowed
                || rule->ruleID() == RTBValidatorRule::DifficultySet
                || rule->ruleID() == RTBValidatorRule::PlayStyleSet)
        {
            mMapDocument->setCurrentObject(mMapDocument->map());
            mMapDocument->emitEditCurrentObject();
        }
    }
}
