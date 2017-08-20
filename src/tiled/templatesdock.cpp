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
#include "preferences.h"
#include "templatemanager.h"
#include "tmxmapformat.h"
#include "utils.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>

using namespace Tiled;
using namespace Tiled::Internal;

TemplatesDock::TemplatesDock(QWidget *parent):
    QDockWidget(parent),
    mTemplatesView(new TemplatesView),
    mNewTemplateGroup(new QAction(this)),
    mOpenTemplateGroup(new QAction(this))
{
    setObjectName(QLatin1String("TemplatesDock"));

    QWidget *widget = new QWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mTemplatesView);

    mNewTemplateGroup->setIcon(QIcon(QLatin1String(":/images/16x16/document-new.png")));
    Utils::setThemeIcon(mNewTemplateGroup, "document-new");
    connect(mNewTemplateGroup, &QAction::triggered, this, &TemplatesDock::newTemplateGroup);

    QToolBar *toolBar = new QToolBar;
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setIconSize(Utils::smallIconSize());

    mOpenTemplateGroup->setIcon(QIcon(QLatin1String(":/images/16x16/document-open.png")));
    Utils::setThemeIcon(mOpenTemplateGroup, "document-open");
    connect(mOpenTemplateGroup, &QAction::triggered, this, &TemplatesDock::openTemplateGroup);

    toolBar->addAction(mNewTemplateGroup);
    toolBar->addAction(mOpenTemplateGroup);

    layout->addWidget(toolBar);
    setWidget(widget);
    retranslateUi();

    // Retrieve saved template groups
    Preferences *prefs = Preferences::instance();
    QString documentsFileName = prefs->templateDocumentsFile();

    TemplateDocuments templateDocuments;

    TemplateDocumentsSerializer templateDocumentsSerializer;
    templateDocumentsSerializer.readTemplateDocuments(documentsFileName, templateDocuments);

    auto model = ObjectTemplateModel::instance();
    model->setTemplateDocuments(templateDocuments);

    mTemplatesView->setModel(model);

    connect(mTemplatesView, &TemplatesView::currentTemplateChanged,
            this, &TemplatesDock::currentTemplateChanged);

    connect(mTemplatesView->model(), &ObjectTemplateModel::dataChanged,
            mTemplatesView, &TemplatesView::applyTemplateGroups);
    connect(mTemplatesView->model(), &ObjectTemplateModel::rowsInserted,
            mTemplatesView, &TemplatesView::applyTemplateGroups);
}

TemplatesDock::~TemplatesDock()
{
    ObjectTemplateModel::deleteInstance();
}

void TemplatesDock::newTemplateGroup()
{
    FormatHelper<TemplateGroupFormat> helper(FileFormat::ReadWrite);
    QString filter = helper.filter();
    QString selectedFilter = TtxTemplateGroupFormat().nameFilter();

    Preferences *prefs = Preferences::instance();
    QString suggestedFileName = prefs->lastPath(Preferences::TemplateDocumentsFile);
    suggestedFileName += tr("/untitled.ttx");

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    suggestedFileName,
                                                    filter,
                                                    &selectedFilter);

    if (fileName.isEmpty())
        return;

    auto templateGroup = new TemplateGroup();
    templateGroup->setName(QFileInfo(fileName).baseName());
    templateGroup->setFileName(fileName);
    QScopedPointer<TemplateGroupDocument>
        templateGroupDocument(new TemplateGroupDocument(templateGroup));

    TemplateGroupFormat *format = helper.formatByNameFilter(selectedFilter);
    templateGroup->setFormat(format);

    QString error;
    if (!templateGroupDocument->save(fileName, &error)) {
        QMessageBox::critical(this, tr("Error Creating Template Group"), error);
        return;
    }

    auto model = ObjectTemplateModel::instance();
    model->addNewDocument(templateGroupDocument.take());

    prefs->setLastPath(Preferences::TemplateDocumentsFile,
                       QFileInfo(fileName).path());
}

void TemplatesDock::openTemplateGroup()
{
    FormatHelper<TemplateGroupFormat> helper(FileFormat::ReadWrite);
    QString filter = helper.filter();

    Preferences *prefs = Preferences::instance();
    QString suggestedFileName = prefs->lastPath(Preferences::TemplateDocumentsFile);
    QString selectedFilter = TtxTemplateGroupFormat().nameFilter();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Template Group"),
                                                    suggestedFileName,
                                                    filter,
                                                    &selectedFilter);

    if (fileName.isEmpty())
        return;

    if (TemplateManager::instance()->findTemplateGroup(fileName)) {
        QMessageBox::information(this, tr("Open Template Group"),
                                 tr("This template group has been already opened."));
        return;
    }

    auto document = TemplateGroupDocument::load(fileName);

    if (!document) {
        QMessageBox::warning(this, tr("Open Template Group"),
                             tr("Couldn't open this template group."));
        return;
    }

    ObjectTemplateModel::instance()->addDocument(document);

    prefs->setLastPath(Preferences::TemplateDocumentsFile,
                       QFileInfo(fileName).path());
}

void TemplatesDock::retranslateUi()
{
    setWindowTitle(tr("Templates"));
    mNewTemplateGroup->setText(tr("New Template Group"));
    mOpenTemplateGroup->setText(tr("Open Template Group"));
}

TemplatesView::TemplatesView(QWidget *parent)
    : QTreeView(parent)
{
    setUniformRowHeights(true);
    setHeaderHidden(true);

    connect(this, &QAbstractItemView::pressed, this, &TemplatesView::onPressed);

    setSelectionBehavior(QAbstractItemView::SelectRows);

    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
}

void TemplatesView::applyTemplateGroups()
{
    auto model = ObjectTemplateModel::instance();
    auto templateDocuments = model->templateDocuments();

    Preferences *prefs = Preferences::instance();

    QString templateDocumentsFile = prefs->templateDocumentsFile();
    QDir templateDocumentsDir = QFileInfo(templateDocumentsFile).dir();

    if (!templateDocumentsDir.exists())
        templateDocumentsDir.mkpath(QLatin1String("."));

    TemplateDocumentsSerializer serializer;
    if (!serializer.writeTemplateDocuments(templateDocumentsFile, templateDocuments)) {
        QMessageBox::critical(this, tr("Error Writing Template Groups"),
                              tr("Error writing to %1:\n%2")
                              .arg(prefs->templateDocumentsFile(),
                                   serializer.errorString()));
    }
}

QSize TemplatesView::sizeHint() const
{
    return Utils::dpiScaled(QSize(130, 100));
}

void TemplatesView::onPressed(const QModelIndex &index)
{
    auto model = ObjectTemplateModel::instance();

    if (ObjectTemplate *objectTemplate = model->toObjectTemplate(index))
        emit currentTemplateChanged(objectTemplate);
}
