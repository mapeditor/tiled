/*
 * preferencesdialog.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include "languagemanager.h"
#include "objecttypesmodel.h"
#include "preferences.h"
#include "utils.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyledItemDelegate>

#ifndef QT_NO_OPENGL
#include <QGLFormat>
#endif

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

class ColorDelegate : public QStyledItemDelegate
{
public:
    ColorDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    { }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &,
                   const QModelIndex &) const;
};

} // namespace Internal
} // namespace Tiled


void ColorDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    const QVariant displayData =
            index.model()->data(index, ObjectTypesModel::ColorRole);
    const QColor color = displayData.value<QColor>();
    const QRect rect = option.rect.adjusted(4, 4, -4, -4);

    const QPen linePen(color, 2);
    const QPen shadowPen(Qt::black, 2);

    QColor brushColor = color;
    brushColor.setAlpha(50);
    const QBrush fillBrush(brushColor);

    // Draw the shadow
    painter->setPen(shadowPen);
    painter->setBrush(QBrush());
    painter->drawRect(rect.translated(QPoint(1, 1)));

    painter->setPen(linePen);
    painter->setBrush(fillBrush);
    painter->drawRect(rect);
}

QSize ColorDelegate::sizeHint(const QStyleOptionViewItem &,
                              const QModelIndex &) const
{
    return QSize(50, 20);
}


PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::PreferencesDialog),
    mLanguages(LanguageManager::instance()->availableLanguages())
{
    mUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

#ifndef QT_NO_OPENGL
    mUi->openGL->setEnabled(QGLFormat::hasOpenGL());
#else
    mUi->openGL->setEnabled(false);
#endif

    foreach (const QString &name, mLanguages) {
        QLocale locale(name);
        QString string = QString(QLatin1String("%1 (%2)"))
            .arg(QLocale::languageToString(locale.language()))
            .arg(QLocale::countryToString(locale.country()));
        mUi->languageCombo->addItem(string, name);
    }

    mUi->languageCombo->model()->sort(0);
    mUi->languageCombo->insertItem(0, tr("System default"));

    mObjectTypesModel = new ObjectTypesModel(this);
    mUi->objectTypesTable->setModel(mObjectTypesModel);
    mUi->objectTypesTable->setItemDelegateForColumn(1, new ColorDelegate(this));

    QHeaderView *horizontalHeader = mUi->objectTypesTable->horizontalHeader();
    horizontalHeader->setResizeMode(QHeaderView::Stretch);

    Utils::setThemeIcon(mUi->addObjectTypeButton, "add");
    Utils::setThemeIcon(mUi->removeObjectTypeButton, "remove");

    fromPreferences();

    connect(mUi->languageCombo, SIGNAL(currentIndexChanged(int)),
            SLOT(languageSelected(int)));
    connect(mUi->openGL, SIGNAL(toggled(bool)), SLOT(useOpenGLToggled(bool)));
    connect(mUi->gridColor, SIGNAL(colorChanged(QColor)),
            Preferences::instance(), SLOT(setGridColor(QColor)));

    connect(mUi->objectTypesTable->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(selectedObjectTypesChanged()));
    connect(mUi->objectTypesTable, SIGNAL(doubleClicked(QModelIndex)),
            SLOT(objectTypeIndexClicked(QModelIndex)));
    connect(mUi->addObjectTypeButton, SIGNAL(clicked()),
            SLOT(addObjectType()));
    connect(mUi->removeObjectTypeButton, SIGNAL(clicked()),
            SLOT(removeSelectedObjectTypes()));
    connect(mUi->importObjectTypesButton, SIGNAL(clicked()),
            SLOT(importObjectTypes()));
    connect(mUi->exportObjectTypesButton, SIGNAL(clicked()),
            SLOT(exportObjectTypes()));

    connect(mObjectTypesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(applyObjectTypes()));
    connect(mObjectTypesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(applyObjectTypes()));

    connect(mUi->autoMapWhileDrawing, SIGNAL(toggled(bool)),
            SLOT(useAutomappingDrawingToggled(bool)));
}

PreferencesDialog::~PreferencesDialog()
{
    toPreferences();
    delete mUi;
}

void PreferencesDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange: {
            const int formatIndex = mUi->layerDataCombo->currentIndex();
            mUi->retranslateUi(this);
            mUi->layerDataCombo->setCurrentIndex(formatIndex);
            mUi->languageCombo->setItemText(0, tr("System default"));
        }
        break;
    default:
        break;
    }
}

void PreferencesDialog::languageSelected(int index)
{
    const QString language = mUi->languageCombo->itemData(index).toString();
    Preferences *prefs = Preferences::instance();
    prefs->setLanguage(language);
}

void PreferencesDialog::useOpenGLToggled(bool useOpenGL)
{
    Preferences::instance()->setUseOpenGL(useOpenGL);
}

void PreferencesDialog::addObjectType()
{
    const int newRow = mObjectTypesModel->objectTypes().size();
    mObjectTypesModel->appendNewObjectType();

    // Select and focus the new row and ensure it is visible
    QItemSelectionModel *sm = mUi->objectTypesTable->selectionModel();
    const QModelIndex newIndex = mObjectTypesModel->index(newRow, 0);
    sm->select(newIndex,
               QItemSelectionModel::ClearAndSelect |
               QItemSelectionModel::Rows);
    sm->setCurrentIndex(newIndex, QItemSelectionModel::Current);
    mUi->objectTypesTable->setFocus();
    mUi->objectTypesTable->scrollTo(newIndex);
}

void PreferencesDialog::selectedObjectTypesChanged()
{
    const QItemSelectionModel *sm = mUi->objectTypesTable->selectionModel();
    mUi->removeObjectTypeButton->setEnabled(sm->hasSelection());
}

void PreferencesDialog::removeSelectedObjectTypes()
{
    const QItemSelectionModel *sm = mUi->objectTypesTable->selectionModel();
    mObjectTypesModel->removeObjectTypes(sm->selectedRows());
}

void PreferencesDialog::objectTypeIndexClicked(const QModelIndex &index)
{
    if (index.column() == 1) {
        QColor color = mObjectTypesModel->objectTypes().at(index.row()).color;
        QColor newColor = QColorDialog::getColor(color, this);
        if (newColor.isValid())
            mObjectTypesModel->setObjectTypeColor(index.row(), newColor);
    }
}

void PreferencesDialog::applyObjectTypes()
{
    Preferences *prefs = Preferences::instance();
    prefs->setObjectTypes(mObjectTypesModel->objectTypes());
}

void PreferencesDialog::importObjectTypes()
{
    Preferences *prefs = Preferences::instance();
    const QString lastPath = prefs->lastPath(Preferences::ObjectTypesFile);
    const QString fileName =
            QFileDialog::getOpenFileName(this, tr("Import Object Types"),
                                         lastPath,
                                         tr("Object Types files (*.xml)"));
    if (fileName.isEmpty())
        return;

    prefs->setLastPath(Preferences::ObjectTypesFile, fileName);

    ObjectTypesReader reader;
    ObjectTypes objectTypes = reader.readObjectTypes(fileName);

    if (reader.errorString().isEmpty()) {
        prefs->setObjectTypes(objectTypes);
        mObjectTypesModel->setObjectTypes(objectTypes);
    } else {
        QMessageBox::critical(this, tr("Error Reading Object Types"),
                              reader.errorString());
    }
}

void PreferencesDialog::exportObjectTypes()
{
    Preferences *prefs = Preferences::instance();
    QString lastPath = prefs->lastPath(Preferences::ObjectTypesFile);

    if (!lastPath.endsWith(QLatin1String(".xml")))
        lastPath.append(QLatin1String("/objecttypes.xml"));

    const QString fileName =
            QFileDialog::getSaveFileName(this, tr("Export Object Types"),
                                         lastPath,
                                         tr("Object Types files (*.xml)"));
    if (fileName.isEmpty())
        return;

    prefs->setLastPath(Preferences::ObjectTypesFile, fileName);

    ObjectTypesWriter writer;
    if (!writer.writeObjectTypes(fileName, prefs->objectTypes())) {
        QMessageBox::critical(this, tr("Error Writing Object Types"),
                              writer.errorString());
    }
}

void PreferencesDialog::fromPreferences()
{
    const Preferences *prefs = Preferences::instance();
    mUi->reloadTilesetImages->setChecked(prefs->reloadTilesetsOnChange());
    mUi->enableDtd->setChecked(prefs->dtdEnabled());
    if (mUi->openGL->isEnabled())
        mUi->openGL->setChecked(prefs->useOpenGL());

    int formatIndex = 0;
    switch (prefs->layerDataFormat()) {
    case MapWriter::XML:
        formatIndex = 0;
        break;
    case MapWriter::Base64:
        formatIndex = 1;
        break;
    case MapWriter::Base64Gzip:
        formatIndex = 2;
        break;
    default:
    case MapWriter::Base64Zlib:
        formatIndex = 3;
        break;
    case MapWriter::CSV:
        formatIndex = 4;
        break;
    }
    mUi->layerDataCombo->setCurrentIndex(formatIndex);

    // Not found (-1) ends up at index 0, system default
    int languageIndex = mUi->languageCombo->findData(prefs->language());
    if (languageIndex == -1)
        languageIndex = 0;
    mUi->languageCombo->setCurrentIndex(languageIndex);
    mUi->gridColor->setColor(prefs->gridColor());
    mUi->autoMapWhileDrawing->setChecked(prefs->automappingDrawing());
    mObjectTypesModel->setObjectTypes(prefs->objectTypes());
}

void PreferencesDialog::toPreferences()
{
    Preferences *prefs = Preferences::instance();

    prefs->setReloadTilesetsOnChanged(mUi->reloadTilesetImages->isChecked());
    prefs->setDtdEnabled(mUi->enableDtd->isChecked());
    prefs->setLayerDataFormat(layerDataFormat());
    prefs->setAutomappingDrawing(mUi->autoMapWhileDrawing->isChecked());
}

MapWriter::LayerDataFormat PreferencesDialog::layerDataFormat() const
{
    switch (mUi->layerDataCombo->currentIndex()) {
    case 0:
        return MapWriter::XML;
    case 1:
        return MapWriter::Base64;
    case 2:
        return MapWriter::Base64Gzip;
    case 3:
    default:
        return MapWriter::Base64Zlib;
    case 4:
        return MapWriter::CSV;
    }
}

void PreferencesDialog::useAutomappingDrawingToggled(bool enabled)
{
    Preferences::instance()->setAutomappingDrawing(enabled);
}
