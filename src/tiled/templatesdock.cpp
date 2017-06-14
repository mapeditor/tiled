/*
 * templatesdock.cpp
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Mohamed Thabet <thabetx@gmail.com>
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

#include "templatesdock.h"

#include "objecttemplatemodel.h"
#include "utils.h"
#include "preferences.h"

#include <QBoxLayout>
#include <QToolBar>

using namespace Tiled;
using namespace Tiled::Internal;

TemplatesDock::TemplatesDock(QWidget *parent):
    QDockWidget(parent),
    mTemplatesView(new TemplatesView)
{
    setObjectName(QLatin1String("TemplatesDock"));

    QWidget *widget = new QWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mTemplatesView);

    mActionNewTemplateGroup = new QAction(this);
    mActionNewTemplateGroup->setIcon(QIcon(QLatin1String(":/images/16x16/document-new.png")));

    QToolBar *toolBar = new QToolBar;
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setIconSize(Utils::smallIconSize());

    toolBar->addAction(mActionNewTemplateGroup);

    layout->addWidget(toolBar);
    setWidget(widget);
    retranslateUi();
}

void TemplatesDock::retranslateUi()
{
    setWindowTitle(tr("Templates"));
}

TemplatesView::TemplatesView(QWidget *parent)
    : QTreeView(parent)
{
    setUniformRowHeights(true);
    setHeaderHidden(true);

    Preferences *prefs = Preferences::instance();
    ObjectTemplateModel *objectTemplateModel = new ObjectTemplateModel(prefs->templateDocuments());

    setModel(objectTemplateModel);

    setSelectionBehavior(QAbstractItemView::SelectRows);
}

QSize TemplatesView::sizeHint() const
{
    return Utils::dpiScaled(QSize(130, 100));
}
