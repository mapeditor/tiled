/*
 * varianteditorfactory.cpp
 * Copyright (C) 2006 Trolltech ASA. All rights reserved. (GPLv2)
 * Copyright 2013-2021, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "varianteditorfactory.h"

#include "fileedit.h"
#include "objectrefedit.h"
#include "textpropertyedit.h"
#include "tilesetdocument.h"
#include "tilesetparametersedit.h"
#include "utils.h"
#include "variantpropertymanager.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QToolButton>

namespace Tiled {

class ResetWidget : public QWidget
{
    Q_OBJECT

public:
    ResetWidget(QtProperty *property, QWidget *editor, QWidget *parent = nullptr);

signals:
    void resetProperty(QtProperty *property);

private:
    void buttonClicked();

    QtProperty *mProperty;
};

ResetWidget::ResetWidget(QtProperty *property, QWidget *editor, QWidget *parent)
    : QWidget(parent)
    , mProperty(property)
{
    auto layout = new QHBoxLayout(this);

    auto resetButton = new QToolButton(this);
    resetButton->setIcon(QIcon(QLatin1String(":/images/16/edit-clear.png")));
    resetButton->setIconSize(Utils::smallIconSize());
    resetButton->setAutoRaise(true);
    resetButton->setToolTip(tr("Reset"));
    Utils::setThemeIcon(resetButton, "edit-clear");

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(editor);
    layout->addWidget(resetButton);

    setFocusProxy(editor);

    connect(resetButton, &QToolButton::clicked, this, &ResetWidget::buttonClicked);
}

void ResetWidget::buttonClicked()
{
    emit resetProperty(mProperty);
}


VariantEditorFactory::~VariantEditorFactory()
{
    // Using QMap::keys here is important because the maps get modified in
    // slotEditorDestroyed.
    qDeleteAll(mFileEditToProperty.keys());
    qDeleteAll(mTilesetEditToProperty.keys());
    qDeleteAll(mTextPropertyEditToProperty.keys());
    qDeleteAll(mObjectRefEditToProperty.keys());
    qDeleteAll(mComboBoxToProperty.keys());
}

void VariantEditorFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
    connect(manager, &QtVariantPropertyManager::valueChanged,
            this, &VariantEditorFactory::slotPropertyChanged);
    connect(manager, &QtVariantPropertyManager::attributeChanged,
            this, &VariantEditorFactory::slotPropertyAttributeChanged);
    QtVariantEditorFactory::connectPropertyManager(manager);
}

QWidget *VariantEditorFactory::createEditor(QtVariantPropertyManager *manager,
                                            QtProperty *property,
                                            QWidget *parent)
{
    const int type = manager->propertyType(property);
    QWidget *editor = nullptr;

    if (type == filePathTypeId()) {
        auto fileEdit = new FileEdit(parent);
        FilePath filePath = manager->value(property).value<FilePath>();
        fileEdit->setFileUrl(filePath.url);
        fileEdit->setFilter(manager->attributeValue(property, QLatin1String("filter")).toString());
        fileEdit->setIsDirectory(manager->attributeValue(property, QLatin1String("directory")).toBool());
        mCreatedFileEdits[property].append(fileEdit);
        mFileEditToProperty[fileEdit] = property;

        connect(fileEdit, &FileEdit::fileUrlChanged,
                this, &VariantEditorFactory::fileEditFileUrlChanged);
        connect(fileEdit, &QObject::destroyed,
                this, &VariantEditorFactory::slotEditorDestroyed);

        editor = fileEdit;
    } else if (type == VariantPropertyManager::displayObjectRefTypeId()) {
        auto objectRefEdit = new ObjectRefEdit(parent);
        auto objectRef = manager->value(property).value<DisplayObjectRef>();
        objectRefEdit->setValue(objectRef);
        mCreatedObjectRefEdits[property].append(objectRefEdit);
        mObjectRefEditToProperty[objectRefEdit] = property;

        connect(objectRefEdit, &ObjectRefEdit::valueChanged,
                this, &VariantEditorFactory::objectRefEditValueChanged);
        connect(objectRefEdit, &QObject::destroyed,
                this, &VariantEditorFactory::slotEditorDestroyed);

        editor = objectRefEdit;
    } else if (type == VariantPropertyManager::tilesetParametersTypeId()) {
        auto tilesetEdit = new TilesetParametersEdit(parent);
        tilesetEdit->setTilesetDocument(manager->value(property).value<TilesetDocument*>());
        mCreatedTilesetEdits[property].append(tilesetEdit);
        mTilesetEditToProperty[tilesetEdit] = property;

        connect(tilesetEdit, &QObject::destroyed,
                this, &VariantEditorFactory::slotEditorDestroyed);

        editor = tilesetEdit;
    } else if (type == QMetaType::QString) {
        bool multiline = manager->attributeValue(property, QLatin1String("multiline")).toBool();
        QStringList suggestions = manager->attributeValue(property, QLatin1String("suggestions")).toStringList();

        if (multiline) {
            auto textEdit = new TextPropertyEdit(parent);
            textEdit->setText(manager->value(property).toString());
            mCreatedTextPropertyEdits[property].append(textEdit);
            mTextPropertyEditToProperty[textEdit] = property;

            connect(textEdit, &TextPropertyEdit::textChanged,
                    this, &VariantEditorFactory::textPropertyEditTextChanged);
            connect(textEdit, &QObject::destroyed,
                    this, &VariantEditorFactory::slotEditorDestroyed);

            editor = textEdit;
        } else if (!suggestions.isEmpty()) {
            auto comboBox = new QComboBox(parent);
            comboBox->setEditable(true);
            comboBox->addItems(suggestions);
            comboBox->setCurrentText(manager->value(property).toString());
            mCreatedComboBoxes[property].append(comboBox);
            mComboBoxToProperty[comboBox] = property;

            connect(comboBox, &QComboBox::currentTextChanged,
                    this, &VariantEditorFactory::comboBoxPropertyEditTextChanged);
            connect(comboBox, &QObject::destroyed,
                    this, &VariantEditorFactory::slotEditorDestroyed);

            editor = comboBox;
        }
    }

    if (!editor)
        editor = QtVariantEditorFactory::createEditor(manager, property, parent);

    if (type == QMetaType::QColor || type == VariantPropertyManager::displayObjectRefTypeId() || property->isModified()) {
        // Allow resetting color and object reference properties, or allow
        // unsetting a class member (todo: resolve conflict...).
        auto resetWidget = new ResetWidget(property, editor, parent);
        connect(resetWidget, &ResetWidget::resetProperty,
                this, &VariantEditorFactory::resetProperty);
        editor = resetWidget;
    }

    return editor;
}

void VariantEditorFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
{
    disconnect(manager, &QtVariantPropertyManager::valueChanged,
               this, &VariantEditorFactory::slotPropertyChanged);
    disconnect(manager, &QtVariantPropertyManager::attributeChanged,
               this, &VariantEditorFactory::slotPropertyAttributeChanged);
    QtVariantEditorFactory::disconnectPropertyManager(manager);
}

void VariantEditorFactory::slotPropertyChanged(QtProperty *property,
                                               const QVariant &value)
{
    if (mCreatedFileEdits.contains(property)) {
        for (FileEdit *edit : std::as_const(mCreatedFileEdits)[property]) {
            FilePath filePath = value.value<FilePath>();
            edit->setFileUrl(filePath.url);
        }
    }
    else if (mCreatedTilesetEdits.contains(property)) {
        for (TilesetParametersEdit *edit : std::as_const(mCreatedTilesetEdits)[property])
            edit->setTilesetDocument(value.value<TilesetDocument*>());
    }
    else if (mCreatedTextPropertyEdits.contains(property)) {
        for (TextPropertyEdit *edit : std::as_const(mCreatedTextPropertyEdits)[property])
            edit->setText(value.toString());
    }
    else if (mCreatedComboBoxes.contains(property)) {
        for (QComboBox *comboBox : std::as_const(mCreatedComboBoxes)[property])
            comboBox->setCurrentText(value.toString());
    }
    else if (mCreatedObjectRefEdits.contains(property)) {
        for (ObjectRefEdit *objectRefEdit : std::as_const(mCreatedObjectRefEdits)[property])
            objectRefEdit->setValue(value.value<DisplayObjectRef>());
    }
}

void VariantEditorFactory::slotPropertyAttributeChanged(QtProperty *property,
                                                        const QString &attribute,
                                                        const QVariant &value)
{
    if (mCreatedFileEdits.contains(property)) {
        if (attribute == QLatin1String("filter")) {
            for (FileEdit *edit : std::as_const(mCreatedFileEdits)[property])
                edit->setFilter(value.toString());
        } else if (attribute == QLatin1String("directory")) {
            for (FileEdit *edit : std::as_const(mCreatedFileEdits)[property])
                edit->setIsDirectory(value.toBool());
        }
    }
    else if (mCreatedComboBoxes.contains(property)) {
        if (attribute == QLatin1String("suggestions")) {
            for (QComboBox *comboBox: std::as_const(mCreatedComboBoxes)[property]) {
                comboBox->clear();
                comboBox->addItems(value.toStringList());
            }
        }
    }
    // changing of "multiline" attribute currently not supported
}

void VariantEditorFactory::fileEditFileUrlChanged(const QUrl &value)
{
    auto fileEdit = qobject_cast<FileEdit*>(sender());
    Q_ASSERT(fileEdit);

    if (QtProperty *property = mFileEditToProperty.value(fileEdit)) {
        QtVariantPropertyManager *manager = propertyManager(property);
        if (!manager)
            return;
        manager->setValue(property, QVariant::fromValue(FilePath { value }));
    }
}

void VariantEditorFactory::textPropertyEditTextChanged(const QString &value)
{
    auto textPropertyEdit = qobject_cast<TextPropertyEdit*>(sender());
    Q_ASSERT(textPropertyEdit);

    if (QtProperty *property = mTextPropertyEditToProperty.value(textPropertyEdit)) {
        QtVariantPropertyManager *manager = propertyManager(property);
        if (!manager)
            return;
        manager->setValue(property, value);
    }
}

void VariantEditorFactory::comboBoxPropertyEditTextChanged(const QString &value)
{
    auto comboBox = qobject_cast<QComboBox*>(sender());
    Q_ASSERT(comboBox);

    if (QtProperty *property = mComboBoxToProperty.value(comboBox)) {
        QtVariantPropertyManager *manager = propertyManager(property);
        if (!manager)
            return;
        manager->setValue(property, value);
    }
}

void VariantEditorFactory::objectRefEditValueChanged(const DisplayObjectRef &value)
{
    auto objectRefEdit = qobject_cast<ObjectRefEdit*>(sender());
    Q_ASSERT(objectRefEdit);
    if (QtProperty *property = mObjectRefEditToProperty.value(objectRefEdit)) {
        QtVariantPropertyManager *manager = propertyManager(property);
        if (!manager)
            return;
        manager->setValue(property, QVariant::fromValue(value));
    }
}

void VariantEditorFactory::slotEditorDestroyed(QObject *object)
{
    // Check if it was an ObjectRefEdit
    {
        auto objectRefEdit = static_cast<ObjectRefEdit*>(object);

        if (QtProperty *property = mObjectRefEditToProperty.value(objectRefEdit)) {
            mObjectRefEditToProperty.remove(objectRefEdit);
            mCreatedObjectRefEdits[property].removeAll(objectRefEdit);
            if (mCreatedObjectRefEdits[property].isEmpty())
                mCreatedObjectRefEdits.remove(property);
            return;
        }
    }

    // Check if it was a FileEdit
    {
        auto fileEdit = static_cast<FileEdit*>(object);

        if (QtProperty *property = mFileEditToProperty.value(fileEdit)) {
            mFileEditToProperty.remove(fileEdit);
            mCreatedFileEdits[property].removeAll(fileEdit);
            if (mCreatedFileEdits[property].isEmpty())
                mCreatedFileEdits.remove(property);
            return;
        }
    }

    // Check if it was a TilesetParametersEdit
    {
        auto tilesetEdit = static_cast<TilesetParametersEdit*>(object);

        if (QtProperty *property = mTilesetEditToProperty.value(tilesetEdit)) {
            mTilesetEditToProperty.remove(tilesetEdit);
            mCreatedTilesetEdits[property].removeAll(tilesetEdit);
            if (mCreatedTilesetEdits[property].isEmpty())
                mCreatedTilesetEdits.remove(property);
            return;
        }
    }

    // Check if it was a TextPropertyEdit
    {
        auto textPropertyEdit = static_cast<TextPropertyEdit*>(object);

        if (QtProperty *property = mTextPropertyEditToProperty.value(textPropertyEdit)) {
            mTextPropertyEditToProperty.remove(textPropertyEdit);
            mCreatedTextPropertyEdits[property].removeAll(textPropertyEdit);
            if (mCreatedTextPropertyEdits[property].isEmpty())
                mCreatedTextPropertyEdits.remove(property);
            return;
        }
    }

    // Check if it was a QComboBox
    {
        auto comboBox = static_cast<QComboBox*>(object);

        if (QtProperty *property = mComboBoxToProperty.value(comboBox)) {
            mComboBoxToProperty.remove(comboBox);
            mCreatedComboBoxes[property].removeAll(comboBox);
            if (mCreatedComboBoxes[property].isEmpty())
                mCreatedComboBoxes.remove(property);
            return;
        }
    }
}

} // namespace Tiled

#include "varianteditorfactory.moc"
#include "moc_varianteditorfactory.cpp"
