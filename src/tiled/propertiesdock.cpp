/*
 * propertiesdock.cpp
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "propertiesdock.h"

#include "propertieswidget.h"

#include <QEvent>

namespace Tiled {

PropertiesDock::PropertiesDock(QWidget *parent)
    : QDockWidget(parent)
    , mPropertiesWidget(new PropertiesWidget(this))
{
    setObjectName(QLatin1String("propertiesDock"));
    setWidget(mPropertiesWidget);

    connect(mPropertiesWidget, &PropertiesWidget::bringToFront,
            this, &PropertiesDock::bringToFront);

    retranslateUi();
}

void PropertiesDock::setDocument(Document *document)
{
    mPropertiesWidget->setDocument(document);
}

void PropertiesDock::selectCustomProperty(const QString &name)
{
    bringToFront();
    mPropertiesWidget->selectCustomProperty(name);
}

bool PropertiesDock::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }

    return QDockWidget::event(event);
}

void PropertiesDock::bringToFront()
{
    show();
    raise();
    mPropertiesWidget->setFocus();
}

void PropertiesDock::retranslateUi()
{
    setWindowTitle(tr("Properties"));
}

} // namespace Tiled

#include "moc_propertiesdock.cpp"
